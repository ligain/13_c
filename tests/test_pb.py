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
        {"device": {"type": "gaid", "id": "e7e"}, "lat": 42, "lon": -42, "apps": [1, 2]},
        {"device": {"type": "gaid", "id": "e7e1a"}, "lat": 42, "lon": -42, "apps": []},
        {"device": {"type": "gaid", "id": "e7e1a5"}, "apps": [1]},
        {"device": {"type": "gaid", "id": "e7e1558c0d7d"}, "apps": [1]},
    ]

    def tearDown(self):
        os.remove(TEST_FILE)

    def test_write(self):
        print("deviceapps before: {}".format(self.deviceapps))
        bytes_written = pb.deviceapps_xwrite_pb(self.deviceapps, TEST_FILE)
        print("bytes_written: {}".format(bytes_written))
        print("deviceapps after: {}".format(self.deviceapps))
        self.assertTrue(bytes_written > 0)
        # check magic, type, etc.

    @unittest.skip("Optional problem")
    def test_read(self):
        bytes_written = pb.deviceapps_xwrite_pb(self.deviceapps, TEST_FILE)
        print("bytes_written: {}".format(bytes_written))
        for i, d in enumerate(pb.deviceapps_xread_pb(TEST_FILE)):
            # self.assertEqual(d, self.deviceapps[i])
            # print("sourse dict: {}".format(d))
            # import pdb; pdb.set_trace()
            print("index: {}".format(i))
            print("parsed dict: {}".format(d))
        # print("deviceapps: {}".format(self.deviceapps))
