local class = require 'pl.class'
local HilscherBluetooth = class()


function HilscherBluetooth:_init(tPlugin)
  self.BT_COMMAND_ReadDeviceInfo = ${BT_COMMAND_ReadDeviceInfo}
  self.BT_COMMAND_UpdateMAC = ${BT_COMMAND_UpdateMAC}

  self.__tLog = tLog
  self.__tPlugin = tPlugin
  self.__aAttr = nil

  local vstruct = require 'vstruct'

  self.tDeviceInfo = vstruct.compile([[
    strFirmwareVersion:z16
    strHardwareVersion:z16
    tBluetoothSerial: {
      aucChipID: {
        4*u1
      }
      aucUniqueDeviceSerialNumber: {
        8*u1
      }
      aucReserved: {
        4*u1
      }
    }
    aucMAC: {
      6*u1
    }
    aucReserved: {
      2*u1
    }
  ]])

  self.tCmdUpdateMAC = vstruct.compile([[
    ucCmd:u1
    aucMAC: {
      6*u1
    }
  ]])
end



function HilscherBluetooth:initialize()
  local tResult
  local tPlugin = self.__tPlugin
  local aAttr = tester:mbin_open('netx/netx90_hilscher_bluetooth.bin', tPlugin)
  tester:mbin_debug(aAttr)
  tester:mbin_write(tPlugin, aAttr)

  -- Initialize the application.
  local aParameter = 0
  tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
  local ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
  if ulValue~=0 then
    error('Failed to initialize the netX module.')
  else
    self.__aAttr = aAttr
    tResult = true
  end

  return tResult
end



function HilscherBluetooth:getDeviceInfo()
  local tResult


  local aAttr = self.__aAttr
  local tPlugin = self.__tPlugin
  if aAttr==nil then
    error('Not Initialized.')
  elseif tPlugin==nil then
    error('No plugin set.')
  else
    -- Try to read the device info from the bluetooth module.
    local aParameter = {
      self.BT_COMMAND_ReadDeviceInfo
    }
    tester:mbin_set_parameter(tPlugin, aAttr, aParameter)
    ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      error('Failed to read the device info.')
    else
      -- Read the complete device info.
      local strData = tester:stdRead(tPlugin, aAttr.ulParameterStartAddress+13, 56)
      tester:hexdump(strData)
      local tData = self.tDeviceInfo:read(strData)
      tResult = tData
    end
  end

  return tResult
end



function HilscherBluetooth:updateMAC(aucMAC)
  local tResult


  local aAttr = self.__aAttr
  local tPlugin = self.__tPlugin
  if aAttr==nil then
    error('Not Initialized.')
  elseif tPlugin==nil then
    error('No plugin set.')
  else
    -- Try to update the MAC in the bluetooth module.
    local strParameter = self.tCmdUpdateMAC:write{
      ucCmd = self.BT_COMMAND_UpdateMAC,
      aucMAC = aucMAC
    }
    tester:mbin_set_parameter(tPlugin, aAttr, strParameter)
    ulValue = tester:mbin_execute(tPlugin, aAttr, aParameter)
    if ulValue~=0 then
      error('Failed to update the MAC.')
    else
      tResult = true
    end
  end

  return tResult
end


return HilscherBluetooth
