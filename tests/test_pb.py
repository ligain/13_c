import os
import unittest

# protoc-c --c_out=. deviceapps.proto
import pb
MAGIC = 0xFFFFFFFF
DEVICE_APPS_TYPE = 1
TEST_FILE = "test.pb.gz"


class TestPB(unittest.TestCase):
    deviceapps = [
        {"device": {"type": "idfa", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7c"},
         "lat": 67.7835424444, "lon": -22.8044005471, "apps": [1, 2, 3, 4]},
        {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "lat": 42, "lon": -42, "apps": [1, 2]},
        {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "lat": 42, "lon": -42, "apps": []},
        {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "apps": [1]},
    ]

    def tearDown(self):
        if os.path.isfile(TEST_FILE):
            os.remove(TEST_FILE)

    def test_write(self):
        bytes_written = pb.deviceapps_xwrite_pb(self.deviceapps, TEST_FILE)
        print("bytes_written: {}".format(bytes_written))
        self.assertTrue(bytes_written > 0)

    def test_read(self):
        pb.deviceapps_xwrite_pb(self.deviceapps, TEST_FILE)
        for i, d in enumerate(pb.deviceapps_xread_pb(TEST_FILE)):
            self.assertEqual(d, self.deviceapps[i])
            print("dict from file: {}".format(d))

    def test_write_bad_device_dict(self):
        bytes_written = pb.deviceapps_xwrite_pb([
            {"device": 42, "apps": [1]},
        ], TEST_FILE)
        self.assertEqual(0, bytes_written)

    def test_write_wrong_apps_types(self):
        with self.assertRaises(ValueError):
            bytes_written = pb.deviceapps_xwrite_pb([
                {
                    "device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"},
                    "apps": ["wrong_type"]
                },
            ], TEST_FILE)
            print("bytes_written: {}".format(bytes_written))

        with self.assertRaises(ValueError):
            bytes_written = pb.deviceapps_xwrite_pb([
                {
                    "device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"},
                    "apps": [1, 2, "wrong_type"]
                },
            ], TEST_FILE)
            print("bytes_written: {}".format(bytes_written))

        with self.assertRaises(ValueError):
            bytes_written = pb.deviceapps_xwrite_pb([
                {
                    "device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"},
                    "apps": [1.0]
                },
            ], TEST_FILE)
            print("bytes_written: {}".format(bytes_written))

    def test_read_bad_file(self):
        with self.assertRaises(OSError):
            pb.deviceapps_xread_pb("wrong_file.pb.gz")
