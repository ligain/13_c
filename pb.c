#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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


// https://github.com/protobuf-c/protobuf-c/wiki/Examples
void example() {
    DeviceApps msg = DEVICE_APPS__INIT;
    DeviceApps__Device device = DEVICE_APPS__DEVICE__INIT;
    void *buf;
    unsigned len;

    char *device_id = "e7e1a50c0ec2747ca56cd9e1558c0d7c";
    char *device_type = "idfa";
    device.has_id = 1;
    device.id.data = (uint8_t*)device_id;
    device.id.len = strlen(device_id);
    device.has_type = 1;
    device.type.data = (uint8_t*)device_type;
    device.type.len = strlen(device_type);
    msg.device = &device;

    msg.has_lat = 1;
    msg.lat = 67.7835424444;
    msg.has_lon = 1;
    msg.lon = -22.8044005471;

    msg.n_apps = 3;
    msg.apps = malloc(sizeof(uint32_t) * msg.n_apps);
    msg.apps[0] = 42;
    msg.apps[1] = 43;
    msg.apps[2] = 44;
    len = device_apps__get_packed_size(&msg);

    buf = malloc(len);
    device_apps__pack(&msg, buf);

    fprintf(stderr,"Writing %d serialized bytes\n",len); // See the length of message
    fwrite(buf, len, 1, stdout); // Write to stdout to allow direct command line piping

    free(msg.apps);
    free(buf);
}

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
            printf("device is not a dict");
            return 1;
        }

        if ((item_device_type = PyDict_GetItemString(item_device, "type")) != NULL) {

            if (PyString_CheckExact(item_device_type)) {
                result->Device.type = PyString_AsString(PyObject_Str(item_device_type));
            } else {
                printf("device type is not a string");
                Py_DECREF(item_device_type);
                Py_DECREF(item_device);
                return 1;
            }
        }

        if ((item_device_id = PyDict_GetItemString(item_device, "id")) != NULL) {

            if (PyString_CheckExact(item_device_id)) {
                result->Device.id = PyString_AsString(PyObject_Str(item_device_id));
            } else {
                printf("item_device_id is not a string");
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

int encode_protobuf_msg(appdevice_t *parsed_dict, void **proto_msg){
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
static PyObject* py_deviceapps_xwrite_pb(PyObject* self, PyObject* args) {
    const char* path;
    PyObject* o = NULL;
    PyObject *iterable = NULL;
    PyObject *item = NULL;
    int return_code = 0;
    appdevice_t *parsed_dict = malloc(sizeof(appdevice_t));
    void *proto_msg = NULL;
    unsigned protobuf_msg_len = 0;

    if (!PyArg_ParseTuple(args, "Os", &o, &path))
        return NULL;

    printf("\nWrite to: %s\n", path);

    if ((iterable = PyObject_GetIter(o)) == NULL)
        return NULL;

    while ((item = PyIter_Next(iterable)) != NULL) {

        if (!PyDict_CheckExact(item))
            continue;

        printf("Start parsing dict: %s\n", PyString_AsString(PyObject_Str(item)));

        return_code = convert_dict_to_struct(item, parsed_dict);
        printf("return_code: %d\n", return_code);
        printf("parsed dict type: %s\n", parsed_dict->Device.type);
        printf("parsed dict id: %s\n", parsed_dict->Device.id);
        printf("parsed dict lat: %f\n", parsed_dict->lat);
        printf("parsed dict lon: %f\n", parsed_dict->lon);
        printf("parsed dict app_n: %d\n", parsed_dict->apps_n);
        printf("parsed dict app[0]: %d\n", parsed_dict->apps[0]);

        if (return_code) {
            printf("Error. Skip dict: %s \n", PyString_AsString(PyObject_Str(item)));
            Py_DECREF(item);
            item = NULL;
            continue;
        }
        protobuf_msg_len = encode_protobuf_msg(parsed_dict, &proto_msg);
        printf("################# protobuf_msg_len: %d\n", protobuf_msg_len);
        printf("proto_msg: %p\n", proto_msg);
        fwrite(proto_msg, protobuf_msg_len, 1, stdout);
        printf("\n");

        Py_DECREF(item);
        item = NULL;
        free(proto_msg);
        proto_msg = NULL;
    }
    free(parsed_dict);
    parsed_dict = NULL;

    Py_DECREF(iterable);
    Py_RETURN_NONE;
}

// Unpack only messages with type == DEVICE_APPS_TYPE
// Return iterator of Python dicts
static PyObject* py_deviceapps_xread_pb(PyObject* self, PyObject* args) {
    const char* path;

    if (!PyArg_ParseTuple(args, "s", &path))
        return NULL;

    printf("Read from: %s\n", path);
    Py_RETURN_NONE;
}


static PyMethodDef PBMethods[] = {
     {"deviceapps_xwrite_pb", py_deviceapps_xwrite_pb, METH_VARARGS, "Write serialized protobuf to file fro iterator"},
     {"deviceapps_xread_pb", py_deviceapps_xread_pb, METH_VARARGS, "Deserialize protobuf from file, return iterator"},
     {NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC initpb(void) {
     (void) Py_InitModule("pb", PBMethods);
}
