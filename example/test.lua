require 'muhkuh_cli_init'
local pl = require'pl.import_into'()
local hibt = require 'hilscher_bluetooth'

-- Create a log target.
local tLogWriter = require 'log.writer.console.color'.new()
local tLog = require "log".new(
  -- maximum log level
  'debug',
  tLogWriter,
  -- Formatter
  require "log.formatter.format".new()
)

_G.tester = require("tester_cli")(tLog)

-- Connect to the netX90 COM side.
local tPlugin = tester:getCommonPlugin()
if tPlugin==nil then
  error("No plugin selected, nothing to do!")
end

local tHiBt = hibt(tPlugin)
tHiBt:initialize()

-- Set a new MAC.
local aucMAC = { 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd7 }
local tResult = tHiBt:updateMAC(aucMAC)
if tResult~=true then
  error('Failed to update the MAC.')
end

-- Read the device info.
local tDeviceInfo = tHiBt:getDeviceInfo()
if tDeviceInfo==nil then
  error('Failed to read the device info.')
end
tLog.info('Bluetooth firmware version: "%s"', tDeviceInfo.strFirmwareVersion)
tLog.info('Bluetooth hardware version: "%s"', tDeviceInfo.strHardwareVersion)
tLog.info('Bluetooth chip identifier:  %s', pl.tablex.reduce(function(a, b) return string.format('%s%02x', a, b) end, tDeviceInfo.tBluetoothSerial.aucChipID, ''))
tLog.info('Bluetooth unique number:    %s', pl.tablex.reduce(function(a, b) return string.format('%s%02x', a, b) end, tDeviceInfo.tBluetoothSerial.aucUniqueDeviceSerialNumber, ''))
tLog.info('Bluetooth public MAC:       %s', table.concat(pl.tablex.map(function(a) return string.format('%02x', a) end, tDeviceInfo.aucMAC), ':'))


-----------------------------------------------------------------------------

print("")
print(" #######  ##    ## ")
print("##     ## ##   ##  ")
print("##     ## ##  ##   ")
print("##     ## #####    ")
print("##     ## ##  ##   ")
print("##     ## ##   ##  ")
print(" #######  ##    ## ")
print("")

-- disconnect the plugin
tPlugin:Disconnect()
