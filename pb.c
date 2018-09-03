#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>
#include "deviceapps.pb-c.h"

#define MAGIC  0xFFFFFFFF
#define DEVICE_APPS_TYPE 1
#define MAX_APPS_IDS 20

typedef struct pbheader_s {
    uint32_t magic;
    uint16_t type;
    uint16_t length;
} pbheader_t;

struct appdevice_s {
    struct device {
        char *type;
        char *id;
    } Device;
    double lat;
    double lon;
    uint8_t apps_n; // number of items in apps[]
    uint8_t apps[MAX_APPS_IDS];
};

typedef struct appdevice_s appdevice_t;

#define PBHEADER_INIT {MAGIC, 0, 0}


int convert_dict_to_struct(PyObject *item, appdevice_t *result){
    PyObject *item_device = NULL;
    PyObject *item_device_type = NULL;
    PyObject *item_device_id = NULL;
    PyObject *item_lat = NULL;
    PyObject *item_lon = NULL;
    PyObject *item_app = NULL;
    PyObject *item_app_id = NULL;

    if ((item_device = PyDict_GetItemString(item, "device")) != NULL) {

        if (!PyDict_CheckExact(item_device)) {
            printf("device is not a dict\n");
            return 1;
        }

        if ((item_device_type = PyDict_GetItemString(item_device, "type")) != NULL) {

            if (PyString_CheckExact(item_device_type)) {
                result->Device.type = PyString_AsString(PyObject_Str(item_device_type));
            } else {
                printf("device type is not a string\n");
                Py_DECREF(item_device_type);
                Py_DECREF(item_device);
                return 1;
            }
        }

        if ((item_device_id = PyDict_GetItemString(item_device, "id")) != NULL) {

            if (PyString_CheckExact(item_device_id)) {
                result->Device.id = PyString_AsString(PyObject_Str(item_device_id));
            } else {
                printf("item_device id is not a string\n");
                Py_DECREF(item_device_id);
                Py_DECREF(item_device);
                return 1;
            }
        }

    } else {
        printf("Invalid Structure\n");
        Py_DECREF(item_device_id);
        Py_DECREF(item_device_type);
        Py_DECREF(item_device);
        return 1;
    }

    if ((item_lat = PyDict_GetItemString(item, "lat")) != NULL) {

        if (PyObject_TypeCheck(item_lat, &PyFloat_Type) || PyObject_TypeCheck(item_lat, &PyInt_Type))
            result->lat = PyFloat_AsDouble(item_lat);

        Py_DECREF(item_lat);
    } else
        result->lat = 0;

    if ((item_lon = PyDict_GetItemString(item, "lon")) != NULL) {

        if (PyObject_TypeCheck(item_lon, &PyFloat_Type) || PyObject_TypeCheck(item_lon, &PyInt_Type))
            result->lon = PyFloat_AsDouble(item_lon);

        Py_DECREF(item_lon);
    } else
        result->lon = 0;

    if ((item_app = PyDict_GetItemString(item, "apps")) != NULL) {
        if (PyObject_TypeCheck(item_app, &PyList_Type)) {
            int apps_number = (uint8_t) PyList_Size(item_app);
            for (int i = 0; i < apps_number; i++) {
                item_app_id = PyList_GetItem(item_app, i);
                if (!PyObject_TypeCheck(item_app_id, &PyInt_Type)) {
                    apps_number = 0;
                    Py_DECREF(item_app_id);
                    break;
                }
                result->apps[i] = (uint8_t) PyInt_AsSsize_t(item_app_id);
                Py_DECREF(item_app_id);
            }
            result->apps_n = apps_number;

        } else {
            printf("app is not a list\n");
            result->apps_n = 0;
        }

        Py_DECREF(item_app);
    } else {
        printf("there is no such item in dist as app\n");
        result->apps_n = 0;
    }

    Py_DECREF(item_device_id);
    Py_DECREF(item_device_type);
    Py_DECREF(item_device);
    return 0;
}

int convert_dict_to_protobuf(appdevice_t *parsed_dict, void **proto_msg){
    DeviceApps msg = DEVICE_APPS__INIT;
    DeviceApps__Device device = DEVICE_APPS__DEVICE__INIT;
    unsigned protobuf_msg_len = 0;

    device.has_id = 1;
    device.id.data = (uint8_t *) parsed_dict->Device.id;
    device.id.len = strlen(parsed_dict->Device.id);

    device.has_type = 1;
    device.type.data = (uint8_t *) parsed_dict->Device.type;
    device.type.len = strlen(parsed_dict->Device.type);
    msg.device = &device;

    if (parsed_dict->lat != 0) {
        msg.has_lat = 1;
        msg.lat = parsed_dict->lat;
    } else {
        msg.has_lat = 0;
    }

    if (parsed_dict->lon != 0) {
        msg.has_lon = 1;
        msg.lon = parsed_dict->lon;
    } else {
        msg.has_lon = 0;
    }

    if (parsed_dict->apps_n) {
        msg.n_apps = parsed_dict->apps_n;
        msg.apps = malloc(sizeof(uint32_t) * msg.n_apps);
        for (int i = 0; i < parsed_dict->apps_n; i++) {
            msg.apps[i] = parsed_dict->apps[i];
        }
    } else {
        msg.n_apps = 0;
        msg.apps = malloc(0);
    }

    protobuf_msg_len = device_apps__get_packed_size(&msg);
    *proto_msg = malloc(protobuf_msg_len);
    device_apps__pack(&msg, *proto_msg);

    free(msg.apps);
    return protobuf_msg_len;
}

// Read iterator of Python dicts
// Pack them to DeviceApps protobuf and write to file with appropriate header
// Return number of written bytes as Python integer
static PyObject *py_deviceapps_xwrite_pb(PyObject *self, PyObject *args) {
    const char *path;
    PyObject *o = NULL;
    PyObject *iterable = NULL;
    PyObject *item = NULL;
    int return_code = 0;
    appdevice_t *parsed_dict = malloc(sizeof(appdevice_t));
    void *proto_msg = NULL;
    unsigned protobuf_msg_len = 0;
    pbheader_t *msg_header = malloc(sizeof(pbheader_t));
    gzFile output_file = NULL;
    unsigned long total_written = 0;

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
            continue;
        }

        printf("Start parsing dict: %s\n", PyString_AsString(PyObject_Str(item)));
        return_code = convert_dict_to_struct(item, parsed_dict);

        if (return_code) {
            printf("Error on parsing dict: %s Skipping... \n", PyString_AsString(PyObject_Str(item)));
            Py_DECREF(item);
            item = NULL;
            continue;
        }
        protobuf_msg_len = convert_dict_to_protobuf(parsed_dict, &proto_msg);

        msg_header->length = protobuf_msg_len;
        msg_header->magic = MAGIC;
        msg_header->type = DEVICE_APPS_TYPE;

        total_written += gzwrite(output_file, msg_header, sizeof(pbheader_t));
        total_written += gzwrite(output_file, proto_msg, protobuf_msg_len);

        Py_DECREF(item);
        item = NULL;

        free(proto_msg);
        proto_msg = NULL;
    }
    free(parsed_dict);
    parsed_dict = NULL;

    free(msg_header);
    msg_header = NULL;

    Py_DECREF(iterable);
    gzclose(output_file);

//    printf("Total bytes written: %li on file: %s\n", total_written, path);
    return Py_BuildValue("k", total_written);
}

int convert_protobuf_to_dict(PyObject *dict, DeviceApps *msg_decoded){
    PyObject *device_dict = NULL;
    PyObject *apps_list = NULL;

    printf("msg_decoded lat: %f\n", msg_decoded->lat);
    printf("msg_decoded lon: %f\n", msg_decoded->lon);
    printf("msg_decoded n_apps: %d\n", msg_decoded->n_apps);
    printf("msg_decoded apps[0]: %d\n", msg_decoded->apps[0]);
    printf("msg_decoded apps[1]: %d\n", msg_decoded->apps[1]);
    printf("msg_decoded apps[2]: %d\n", msg_decoded->apps[2]);
    printf("msg_decoded apps[3]: %d\n", msg_decoded->apps[3]);
    printf("msg_decoded device has id: %d\n", msg_decoded->device->has_id);
    printf("msg_decoded device id len: %li\n", msg_decoded->device->id.len);
    printf("msg_decoded device id: ");
    for (int i = 0; i < msg_decoded->device->id.len; i++) {
        printf("%c", msg_decoded->device->id.data[i]);
    }
    printf("\n");
    printf("msg_decoded device has type: %d\n", msg_decoded->device->has_type);
    printf("msg_decoded device type len: %li\n", msg_decoded->device->type.len);
    printf("msg_decoded device type: ");
    for (int i = 0; i < msg_decoded->device->type.len; i++) {
        printf("%c", msg_decoded->device->type.data[i]);
    }
    printf("\n");

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

    printf("device_dict: ");
    PyObject_Print(device_dict, stdout, 0);
    printf("\n");

    printf("before list\n");
    apps_list = PyList_New(0);
    printf("after list\n");
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

    printf("dict: ");
    PyObject_Print(dict, stdout, 0);
    printf("\n");
    return 0;
}

// Unpack only messages with type == DEVICE_APPS_TYPE
// Return iterator of Python dicts
static PyObject *py_deviceapps_xread_pb(PyObject *self, PyObject *args) {
    const char *path;
    gzFile output_file = NULL;
    PyObject *output_list = PyList_New(0);
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
    while (1) {
        if (gzeof (output_file)) {
            break;
        }
        // read header
        printf("header_buf: %p\n", header_buf);
        printf("header_size: %d\n", header_size);
        read_bytes = gzread(output_file, header_buf, header_size);
        printf("read_bytes: %d\n", read_bytes);
        if (read_bytes > 0) {
            printf("header_buf magic: %x\n", header_buf->magic);
            printf("header_buf type: %d\n", header_buf->type);
            printf("header_buf length: %d\n", header_buf->length);

            // read protobuf message from file
            msg_buf = (uint8_t *) malloc(header_buf->length);
            read_bytes = gzread(output_file, msg_buf, header_buf->length);
            printf("msg read_bytes: %d\n", read_bytes);
            printf("msg start--->");
            for (int i = 0; i < read_bytes; i++) {
                printf("%c", msg_buf[i]);
            }
            printf("<---end of msg\n");

            msg_decoded = device_apps__unpack(NULL, header_buf->length, msg_buf);
            result_dict = PyDict_New();

            convert_protobuf_to_dict(result_dict, msg_decoded);

            device_apps__free_unpacked(msg_decoded, NULL);
            free(msg_buf);
            break;
        } else
            break;
    }

    PyList_Append(output_list, Py_BuildValue("i", 10));
    PyList_Append(output_list, Py_BuildValue("i", 11));
    PyList_Append(output_list, Py_BuildValue("i", 12));

    gzclose(output_file);
    free(header_buf);

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
