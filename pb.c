#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>
#include "deviceapps.pb-c.h"

#define MAGIC  0xFFFFFFFF
#define DEVICE_APPS_TYPE 1

typedef struct pbheader_s {
    uint32_t magic;
    uint16_t type;
    uint16_t length;
} pbheader_t;


// Read iterator of Python dicts
// Pack them to DeviceApps protobuf and write to file with appropriate header
// Return number of written bytes as Python integer
static PyObject *py_deviceapps_xwrite_pb(PyObject *self, PyObject *args) {
    const char *path;
    PyObject *o = NULL;
    PyObject *iterable = NULL;
    PyObject *item = NULL;
    PyObject *item_app_id = NULL;
    PyObject *item_device = NULL;
    PyObject *item_device_type = NULL;
    PyObject *item_device_id = NULL;
    PyObject *item_lat = NULL;
    PyObject *item_lon = NULL;
    PyObject *item_apps = NULL;
    void *proto_msg = NULL;
    pbheader_t *msg_header = malloc(sizeof(pbheader_t));
    gzFile output_file = NULL;
    unsigned long total_written = 0;
    unsigned protobuf_msg_len = 0;

    if (!PyArg_ParseTuple(args, "Os", &o, &path)){
        PyErr_SetString(PyExc_TypeError, "error parsing arguments\n");
        return NULL;
    }

    printf("\nWrite to: %s\n", path);
    output_file = gzopen(path, "wb");
    if (output_file == NULL) {
        PyErr_SetString(PyExc_OSError, "can not create a file\n");
        return NULL;
    }

    if ((iterable = PyObject_GetIter(o)) == NULL) {
        PyErr_SetString(PyExc_TypeError, "first parameter must be an Iterable\n");
        return NULL;
    }

    while ((item = PyIter_Next(iterable)) != NULL) {

        if (!PyDict_CheckExact(item)) {
            printf("Item is not a dict. Skipping... \n");
            Py_DECREF(item);
            continue;
        }

        printf("Start parsing dict: ");
        PyObject_Print(item, stdout, 0);
        printf("\n");

        DeviceApps msg = DEVICE_APPS__INIT;
        DeviceApps__Device device = DEVICE_APPS__DEVICE__INIT;

        item_device = PyDict_GetItemString(item, "device");
        item_device_type = PyDict_GetItemString(item_device, "type");
        item_device_id = PyDict_GetItemString(item_device, "id");

        device.has_id = 1;
        device.id.data = (uint8_t *) PyString_AsString(item_device_id);
        device.id.len = strlen(PyString_AsString(item_device_id));

        device.has_type = 1;
        device.type.data = (uint8_t *) PyString_AsString(item_device_type);
        device.type.len = strlen(PyString_AsString(item_device_type));
        msg.device = &device;

        if ((item_lat = PyDict_GetItemString(item, "lat")) != NULL) {
            msg.has_lat = 1;
            msg.lat = PyFloat_AsDouble(item_lat);
        } else {
            msg.has_lat = 0;
        }

        if ((item_lon = PyDict_GetItemString(item, "lon")) != NULL) {
            msg.has_lon = 1;
            msg.lon = PyFloat_AsDouble(item_lon);
        } else {
            msg.has_lon = 0;
        }

        item_apps = PyDict_GetItemString(item, "apps");
        int apps_number = PyList_Size(item_apps);
        msg.n_apps = apps_number;
        msg.apps = malloc(sizeof(uint32_t) * msg.n_apps);
        for (int i = 0; i < apps_number; i++) {
            item_app_id = PyList_GetItem(item_apps, i);
            if (!PyObject_TypeCheck(item_app_id, &PyInt_Type)) {
                apps_number = 0;
                break;
            }
            msg.apps[i] = (uint8_t) PyInt_AsSsize_t(item_app_id);
        }
        Py_DECREF(item_app_id);

        protobuf_msg_len = device_apps__get_packed_size(&msg);
        proto_msg = malloc(protobuf_msg_len);
        device_apps__pack(&msg, proto_msg);

        free(msg.apps);

        msg_header->length = protobuf_msg_len;
        msg_header->magic = MAGIC;
        msg_header->type = DEVICE_APPS_TYPE;

        total_written += gzwrite(output_file, msg_header, sizeof(pbheader_t));
        total_written += gzwrite(output_file, proto_msg, protobuf_msg_len);

        Py_DECREF(item);

        free(proto_msg);
        proto_msg = NULL;
    }

    gzclose(output_file);
    free(msg_header);
    Py_DECREF(iterable);

    return PyInt_FromLong(total_written);
}

static int convert_protobuf_to_dict(PyObject *dict, DeviceApps *msg_decoded){
    PyObject *device_dict = NULL;
    PyObject *apps_list = NULL;

    if (!msg_decoded->device->has_id) {
        printf("absent device id in protobuf message\n");
        return 1;
    }
    if (!msg_decoded->device->has_type) {
        printf("absent device type in protobuf message\n");
        return 1;
    }
    device_dict = PyDict_New();
    PyDict_SetItemString(device_dict, "id", Py_BuildValue("s#", msg_decoded->device->id.data, msg_decoded->device->id.len));
    PyDict_SetItemString(device_dict, "type", Py_BuildValue("s#", msg_decoded->device->type.data, msg_decoded->device->type.len));

    apps_list = PyList_New(0);
    if (msg_decoded->n_apps) {
        for (int i = 0; i < msg_decoded->n_apps; i++) {
            PyList_Append(apps_list, Py_BuildValue("i", msg_decoded->apps[i]));
        }
    }

    PyDict_SetItemString(dict, "device", device_dict);
    PyDict_SetItemString(dict, "apps", apps_list);
    if (msg_decoded->has_lat)
        PyDict_SetItemString(dict, "lat", Py_BuildValue("d", msg_decoded->lat));
    if (msg_decoded->has_lon)
        PyDict_SetItemString(dict, "lon", Py_BuildValue("d", msg_decoded->lon));

    Py_DECREF(device_dict);
    Py_DECREF(apps_list);
    return 0;
}

// Unpack only messages with type == DEVICE_APPS_TYPE
// Return iterator of Python dicts
static PyObject *py_deviceapps_xread_pb(PyObject *self, PyObject *args) {
    const char *path;
    gzFile output_file = NULL;
    PyObject *output_list = NULL;
    int read_bytes = 0;
    int header_size = sizeof(pbheader_t);
    pbheader_t *header_buf = malloc(header_size);
    uint8_t *msg_buf = NULL;
    DeviceApps *msg_decoded = NULL;
    PyObject *result_dict = NULL;

    if (!PyArg_ParseTuple(args, "s", &path)) {
        free(header_buf);
        return NULL;
    }

    printf("Read from: %s\n", path);
    output_file = gzopen(path, "rb");
    if (output_file == NULL) {
        PyErr_SetString(PyExc_OSError, "can not open file\n");
        free(header_buf);
        return NULL;
    }
    output_list = PyList_New(0);
    while (1) {
        if (gzeof (output_file)) {
            break;
        }
        // read header
        read_bytes = gzread(output_file, header_buf, header_size);
        if (read_bytes > 0) {
            // read protobuf message from file
            msg_buf = (uint8_t *) malloc(header_buf->length);
            read_bytes = gzread(output_file, msg_buf, header_buf->length);
            msg_decoded = device_apps__unpack(NULL, header_buf->length, msg_buf);

            result_dict = PyDict_New();
            int status = convert_protobuf_to_dict(result_dict, msg_decoded);
            if (!status) {
                PyList_Append(output_list, result_dict);
            } else
                printf("error on parsing protobuf message");

            device_apps__free_unpacked(msg_decoded, NULL);
            free(msg_buf);
        } else
            break;
    }

    free(header_buf);
    gzclose(output_file);

    return PySeqIter_New(output_list);
}


static PyMethodDef PBMethods[] = {
     {"deviceapps_xwrite_pb", py_deviceapps_xwrite_pb, METH_VARARGS, "Write serialized protobuf to file fro iterator"},
     {"deviceapps_xread_pb", py_deviceapps_xread_pb, METH_VARARGS, "Deserialize protobuf from file, return iterator"},
     {NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC initpb(void) {
     (void) Py_InitModule("pb", PBMethods);
}
