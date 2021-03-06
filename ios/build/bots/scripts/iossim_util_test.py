# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Unittests for iossim_util.py."""

import iossim_util
import mock
import test_runner
import test_runner_test


class GetiOSSimUtil(test_runner_test.TestCase):
  """Tests for iossim_util.py."""

  def setUp(self):
    super(GetiOSSimUtil, self).setUp()

  def test_get_simulator_runtime_by_version(self):
    """Ensures correctness of filter."""
    self.assertEqual(
        'com.apple.CoreSimulator.SimRuntime.iOS-13-2',
        iossim_util.get_simulator_runtime_by_version(SIMULATORS_LIST, '13.2.2'))

  def test_get_simulator_runtime_by_version_not_found(self):
    """Ensures that SimulatorNotFoundError raises if no runtime."""
    with self.assertRaises(test_runner.SimulatorNotFoundError) as context:
      iossim_util.get_simulator_runtime_by_version(SIMULATORS_LIST, '13.2')
    expected_message = ('Simulator does not exist: Not found '
                        '"13.2" SDK in runtimes')
    self.assertTrue(expected_message in str(context.exception))

  def test_get_simulator_runtime_by_platform(self):
    """Ensures correctness of filter."""
    self.assertEqual(
        'com.apple.CoreSimulator.SimDeviceType.iPhone-11',
        iossim_util.get_simulator_runtime_by_platform(SIMULATORS_LIST,
                                                      'iPhone 11'))

  def test_get_simulator_runtime_by_platform_not_found(self):
    """Ensures that SimulatorNotFoundError raises if no platform."""
    with self.assertRaises(test_runner.SimulatorNotFoundError) as context:
      iossim_util.get_simulator_runtime_by_platform(SIMULATORS_LIST,
                                                    'iPhone XI')
    expected_message = ('Simulator does not exist: Not found device '
                        '"iPhone XI" in devicetypes')
    self.assertTrue(expected_message in str(context.exception))

  def test_get_simulator_runtime_by_device_udid(self):
    """Ensures correctness of filter."""
    self.assertEqual(
        'com.apple.CoreSimulator.SimRuntime.iOS-13-2',
        iossim_util.get_simulator_runtime_by_device_udid(
            'E4E66321-177A-450A-9BA1-488D85B7278E'))

  def test_get_simulator_runtime_by_device_udid_not_found(self):
    """Ensures that SimulatorNotFoundError raises if no device with UDID."""
    with self.assertRaises(test_runner.SimulatorNotFoundError) as context:
      iossim_util.get_simulator_runtime_by_device_udid('non_existing_UDID')
    expected_message = ('Simulator does not exist: Not found simulator with '
                        '"non_existing_UDID" UDID in devices')
    self.assertTrue(expected_message in str(context.exception))

  def test_get_simulator_udids_by_platform_and_sdk(self):
    """Ensures correctness of filter."""
    self.assertEqual(['A4E66321-177A-450A-9BA1-488D85B7278E'],
                     iossim_util.get_simulator_udids_by_platform_and_sdk(
                         'iPhone 11', '13.2.2'))

  def test_get_simulator_udids_by_platform_and_sdk(self):
    """Ensures correctness of filter."""
    self.assertEqual(['A4E66321-177A-450A-9BA1-488D85B7278E'],
                     iossim_util.get_simulator_udids_by_platform_and_sdk(
                         'iPhone 11', '13.2.2'))

  def test_get_simulator_udids_by_platform_and_sdk_not_found(self):
    """Ensures that filter returns empty list if no device with SDK."""
    self.assertEqual([],
                     iossim_util.get_simulator_udids_by_platform_and_sdk(
                         'iPhone 11', '13.1'))

  @mock.patch('subprocess.check_output', autospec=True)
  def test_create_device_by_platform_and_version(self, subprocess_mock):
    """Ensures that command is correct."""
    subprocess_mock.return_value = 'NEW_UDID'
    self.assertEqual(
        'NEW_UDID',
        iossim_util.create_device_by_platform_and_version(
            'iPhone 11', '13.2.2'))
    self.assertEqual([
        'xcrun', 'simctl', 'create', 'iPhone 11 test', 'iPhone 11',
        'com.apple.CoreSimulator.SimRuntime.iOS-13-2'
    ], subprocess_mock.call_args.args[0])

  @mock.patch('subprocess.call', autospec=True)
  def test_delete_simulator_by_udid(self, subprocess_mock):
    """Ensures that command is correct."""
    iossim_util.delete_simulator_by_udid('UDID')
    self.assertEqual(['xcrun', 'simctl', 'delete', 'UDID'],
                     subprocess_mock.call_args.args[0])

  @mock.patch('subprocess.check_call', autospec=True)
  def test_wipe_simulator_by_platform_and_version(self, subprocess_mock):
    """Ensures that command is correct."""
    iossim_util.wipe_simulator_by_udid('A4E66321-177A-450A-9BA1-488D85B7278E')
    self.assertEqual(
        ['xcrun', 'simctl', 'erase', 'A4E66321-177A-450A-9BA1-488D85B7278E'],
        subprocess_mock.call_args.args[0])

  @mock.patch('subprocess.check_output', autospec=True)
  def test_get_home_directory(self, subprocess_mock):
    """Ensures that command is correct."""
    subprocess_mock.return_value = 'HOME_DIRECTORY'
    self.assertEqual('HOME_DIRECTORY',
                     iossim_util.get_home_directory('iPhone 11', '13.2.2'))
    self.assertEqual([
        'xcrun', 'simctl', 'getenv', 'A4E66321-177A-450A-9BA1-488D85B7278E',
        'HOME'
    ], subprocess_mock.call_args.args[0])
