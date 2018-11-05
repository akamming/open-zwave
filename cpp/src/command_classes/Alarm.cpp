//-----------------------------------------------------------------------------
//
//	Alarm.cpp
//
//	Implementation of the Z-Wave COMMAND_CLASS_NOTIFICATIOn (formally COMMAND_CLASS_ALARM)
//
//	Copyright (c) 2010 Mal Lansell <openzwave@lansell.org>
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

// This CommandClass was renamed from ALARM to NOTIFICATION in version 3
// But we cannot rename the class names as we already have a Notification Class used
// for signaling events to the application.



#include "command_classes/CommandClasses.h"
#include "command_classes/Alarm.h"
#include "command_classes/NodeNaming.h"
#include "Defs.h"
#include "Msg.h"
#include "Node.h"
#include "Driver.h"
#include "platform/Log.h"

#include "value_classes/ValueByte.h"
#include "value_classes/ValueBool.h"
#include "value_classes/ValueList.h"
#include "value_classes/ValueString.h"
using namespace OpenZWave;

enum AlarmCmd
{
	AlarmCmd_Get				= 0x04,
	AlarmCmd_Report				= 0x05,
	AlarmCmd_Set				= 0x06,
	// Version 2
	AlarmCmd_SupportedGet		= 0x07,
	AlarmCmd_SupportedReport	= 0x08
};

enum
{
	AlarmIndex_Type = 0,
	AlarmIndex_Level,
	AlarmIndex_SourceNodeId,
	AlarmIndex_Count,
	AlarmIndex_Params = 200
};

enum
{
	Alarm_General = 0,
	Alarm_Smoke,
	Alarm_CarbonMonoxide,
	Alarm_CarbonDioxide,
	Alarm_Heat,
	Alarm_Flood,
	Alarm_Access_Control,
	Alarm_HomeSecurity,
	Alarm_Power_Management,
	Alarm_System,
	Alarm_Emergency,
	Alarm_Clock,
	Alarm_Appliance,
	Alarm_HomeHealth,
	Alarm_Siren,
	Alarm_WaterValve,
	Alarm_Weather,
	Alarm_Irrigation,
	Alarm_Gas,
	Alarm_Count,
	Alarm_Pending = 0xff
};

static char const* c_alarmTypeName[] =
{
		"General",
		"Smoke",
		"Carbon Monoxide",
		"Carbon Dioxide",
		"Heat",
		"Flood",
		"Access Control",
		"Burglar",
		"Power Management",
		"System",
		"Emergency",
		"Clock",
		"Appliance",
		"HomeHealth",
		"Siren",
		"Water Valve",
		"Weather",
		"Irrigation",
		"Gas"
};


enum Alarm_Smoke_Event {
	Alarm_Smoke_Clear = 0x00,
	Alarm_Smoke_Detected_Location = 0x01,
	Alarm_Smoke_Detected_UnknownLocation = 0x02,
	Alarm_Smoke_Alarm_Test = 0x03,
	Alarm_Smoke_Replacement_Required_Unspecified = 0x04,
	Alarm_Smoke_Replacement_Required_End_Of_Life = 0x05,
	Alarm_Smoke_Silenced = 0x06,
	Alarm_Smoke_Maintenance_Required_Periodic_Inspection = 0x07,
	Alarm_Smoke_Maintenance_Required_Dust_In_Device = 0x08,
	Alarm_Smoke_Unknown_Event = 0xFE,
};
enum Alarm_CO_Event {
	Alarm_CO_Clear = 0x00,
	Alarm_CO_Detected_Location = 0x01,
	Alarm_CO_Detected_UnknownLocation = 0x02,
	Alarm_CO_Carbon_Monoxied_Test = 0x03,
	Alarm_CO_Replacement_Required_Unspecified = 0x04,
	Alarm_CO_Replacement_Required_End_Of_Life = 0x05,
	Alarm_CO_Silenced = 0x06,
	Alarm_CO_Maintence_Required_Periodic_Inspection = 0x07,
	Alarm_CO_Unknown_Event = 0xFE,
};
enum Alarm_CO2_Event {
	Alarm_CO2_Clear = 0x00,
	Alarm_CO2_Detected_Location = 0x01,
	Alarm_CO2_Detected_UnknownLocation = 0x02,
	Alarm_CO2_Carbon_Dioxide_Test = 0x03,
	Alarm_CO2_Replacement_Required_Unspecified = 0x04,
	Alarm_CO2_Replacement_Required_End_Of_Life = 0x05,
	Alarm_CO2_Silenced = 0x06,
	Alarm_CO2_Maintence_Required_Periodic_Inspection = 0x07,
	Alarm_CO2_Unknown_Event = 0xFE,
};
enum Alarm_Heat_Event {
	Alarm_Heat_Clear = 0x00,
	Alarm_Heat_OverHeat_Location = 0x01,
	Alarm_Heat_OverHeat_UnknownLocation = 0x02,
	Alarm_Heat_RapidTempRise_Location = 0x03,
	Alarm_Heat_RapidTempRise_UnknownLocation = 0x04,
	Alarm_Heat_UnderHeat_Location = 0x05,
	Alarm_Heat_UnderHeat_UnknownLocation = 0x06,
	Alarm_Heat_Alarm_Test = 0x07,
	Alarm_Heat_Replacement_Required_End_Of_Life = 0x08,
	Alarm_Heat_Silenced = 0x09,
	Alarm_Heat_Maintenance_Required_Dust_In_Device = 0x0A,
	Alarm_Heat_Maintenance_Required_Periodic_Inspection = 0x0B,
	Alarm_Heat_Unknown_Event = 0xFE,
};
enum Alarm_Flood_Event {
	Alarm_Flood_Clear = 0x00,
	Alarm_Flood_Leak_Location = 0x01,
	Alarm_Flood_Leak_UnknownLocation = 0x02,
	Alarm_Flood_Drop_Location = 0x03,
	Alarm_Flood_Drop_UnknownLocation = 0x04,
	Alarm_Flood_Replace_Filter = 0x05,
	Alarm_Flood_Flow = 0x06,
	Alarm_Flood_Pressure = 0x07,
	Alarm_Flood_Unknown_Event = 0xFE,
};
enum Alarm_Access_Control_Event {
	Alarm_Access_Control_Clear = 0x00,
	Alarm_Access_Control_Manual_Lock = 0x01,
	Alarm_Access_Control_Manual_Unlock = 0x02,
	Alarm_Access_Control_RF_Lock = 0x03,
	Alarm_Access_Control_RF_Unlock = 0x04,
	Alarm_Access_Control_Keypad_Lock = 0x05,
	Alarm_Access_Control_Keypad_Unlock = 0x06,
	Alarm_Access_Control_Manual_Not_Fully_Locked = 0x07,
	Alarm_Access_Control_RF_Not_Fully_Locked = 0x08,
	Alarm_Access_Control_Auto_Lock = 0x09,
	Alarm_Access_Control_Auto_Lock_Not_Fully_Locked = 0x0A,
	Alarm_Access_Control_Locked_Jamed = 0x0B,
	Alarm_Access_Control_User_Codes_ALl_Deleted = 0x0C,
	Alarm_Access_Control_User_Codes_Single_Deleted = 0x0D,
	Alarm_Access_Control_User_Codes_Code_Added = 0x0E,
	Alarm_Access_Control_User_Codes_Not_Added_Duplicate = 0x0F,
	Alarm_Access_Control_Keypad_Disabled = 0x10,
	Alarm_Access_Control_Keypad_Busy = 0x11,
	Alarm_Access_Control_New_Program_Code_Entered = 0x12,
	Alarm_Access_Control_User_Codes_Exceeds_Limit = 0x13,
	Alarm_Access_Control_RF_Unlock_Invalid_UserCode = 0x14,
	Alarm_Access_Control_RF_Lock_Invalid_UserCode = 0x15,
	Alarm_Access_Control_Door_Window_Open = 0x16,
	Alarm_Access_Control_Door_Window_Closed = 0x17,
	Alarm_Access_Control_Barrier_Initializing = 0x40,
	Alarm_Access_Control_Barrier_Force_Exceeded = 0x41,
	Alarm_Access_Control_Barrier_Motor_Time_Exceeded = 0x42,
	Alarm_Access_Control_Barrier_Physical_Limits_Exceeded = 0x43,
	Alarm_Access_Control_Barrier_Failed_Operation_UL = 0x44,
	Alarm_Access_Control_Barrier_Unattended_Disabled_UL = 0x45,
	Alarm_Access_Control_Barrier_Malfunction = 0x46,
	Alarm_Access_Control_Barrier_Vacaction_Mode = 0x47,
	Alarm_Access_Control_Barrier_Saftey_Beam_Obstruction = 0x48,
	Alarm_Access_Control_Barrier_Sensor_Not_Detected = 0x49,
	Alarm_Access_Control_Barrier_Battery_Low = 0x4A,
	Alarm_Access_Control_Barrier_Short_Wall_Station_Wires = 0x4B,
	Alarm_Access_Control_Barrier_Associated_Non_ZWave_Device = 0x4C,
	Alarm_Access_Control_Unknown_Event = 0xFE,
};
enum Alarm_HomeSecurity_Event {
	Alarm_HomeSecurity_Clear = 0x00,
	Alarm_HomeSecurity_Intrusion_Location = 0x01,
	Alarm_HomeSecurity_Intrusion_UnknownLocation = 0x02,
	Alarm_HomeSecurity_Intrusion_Tamper_CoverRemoved = 0x03,
	Alarm_HomeSecurity_Intrusion_Tamer_InvalidCode = 0x04,
	Alarm_HomeSecurity_GlassBreakage_Location = 0x05,
	Alarm_HomeSecurity_GlassBreakage_UnknownLocation = 0x06,
	Alarm_HomeSecurity_Motion_Detected_Location = 0x07,
	Alarm_HomeSecurity_Motion_Detected_Location_Unknown = 0x08,
	Alarm_HomeSecurity_Tampering_Product_Moved = 0x09,
	Alarm_HomeSecurity_Unknown_Event = 0xFE,
};
enum Alarm_Power_Management_Event {
	Alarm_Power_Clear = 0x00,
	Alarm_Power_Management_PowerApplied = 0x01,
	Alarm_Power_Management_AC_PowerLost = 0x02,
	Alarm_Power_Management_AC_PowerRestored = 0x03,
	Alarm_Power_Management_Surge = 0x04,
	Alarm_Power_Management_Brownout = 0x05,
	Alarm_Power_Management_Over_Current = 0x06,
	Alarm_Power_Management_Over_Voltage = 0x07,
	Alarm_Power_Management_Over_Load = 0x08,
	Alarm_Power_Management_Load_Error = 0x09,
	Alarm_Power_Management_Replace_Battery_Soon = 0x0A,
	Alarm_Power_Management_Replace_Battery_Now = 0x0B,
	Alarm_Power_Management_Battery_Charging = 0x0C,
	Alarm_Power_Management_Battery_Charged = 0x0D,
	Alarm_Power_Management_Battery_Low = 0x0E,
	Alarm_Power_Management_Battery_Critical = 0x0F,
	Alarm_Power_Management_Unknown_Event = 0xFE,
};
enum Alarm_System_Event {
	Alarm_System_Clear = 0x00,
	Alarm_System_Hardware_Failure = 0x01,
	Alarm_System_Software_Failure = 0x02,
	Alarm_System_Hardware_Failure_Code = 0x03,
	Alarm_System_Software_Failure_Code = 0x04,
	Alarm_System_Heartbeat = 0x05,
	Alarm_System_Tampering = 0x06,
	Alarm_System_Emergency_Shutoff = 0x07,
	Alarm_System_Unknown_Event = 0xFE,
};
enum Alarm_Emergency_Event {
	Alarm_Emergency_Clear = 0x00,
	Alarm_Emergency_Police = 0x01,
	Alarm_Emergency_Fire = 0x02,
	Alarm_Emergency_Medical = 0x03,
	Alarm_Emergency_Unknown_Event = 0xFE,
};
enum Alarm_Clock_Event {
	Alarm_Clock_Clear = 0x00,
	Alarm_Clock_Wakeup = 0x01,
	Alarm_Clock_Timer = 0x02,
	Alarm_Clock_Time_Remaining = 0x03,
	Alarm_Clock_Unknown_Event = 0xFE
};

enum Alarm_Appliance_Event {
	Alarm_Appliance_Clear = 0x00,
	Alarm_Appliance_Program_Started = 0x01,
	Alarm_Appliance_Program_In_Progress = 0x02,
	Alarm_Appliance_Program_Completed = 0x03,
	Alarm_Appliance_Replace_Main_Filter = 0x04,
	Alarm_Appliance_Failure_To_Set_Temp = 0x05,
	Alarm_Appliance_Water_Supply_ok = 0x06,
	Alarm_Appliance_Water_Supply_Failure = 0x07,
	Alarm_Appliance_Boiling = 0x08,
	Alarm_Appliance_Boiling_Failure = 0x09,
	Alarm_Appliance_Washing = 0x0A,
	Alarm_Appliance_Washing_Failure = 0x0B,
	Alarm_Appliance_Rinsing = 0x0C,
	Alarm_Appliance_Rinsing_Failure = 0x0D,
	Alarm_Appliance_Draining = 0x0E,
	Alarm_Appliance_Draining_Failure = 0x0F,
	Alarm_Appliance_Spinning = 0x10,
	Alarm_Appliance_Spinning_Failure = 0x11,
	Alarm_Appliance_Drying = 0x12,
	Alarm_Appliance_Drying_Failure = 0x13,
	Alarm_Appliance_Fan_Failure = 0x14,
	Alarm_Appliance_Compressor_Failure = 0x15,
	Alarm_Appliance_Unknown_Event = 0xFE
};

enum Alarm_HomeHealth {
	Alarm_HomeHealth_Clear = 0x00,
	Alarm_HomeHealth_Leaving_Bed = 0x01,
	Alarm_HomeHealth_Sitting_On_Bed = 0x02,
	Alarm_HomeHealth_Lying_On_Bed = 0x03,
	Alarm_HomeHealth_Posture_Changed = 0x04,
	Alarm_HomeHealth_Sitting_On_Edge_Of_Bed = 0x05,
	Alarm_HomeHealth_Volatile_Organic_Level = 0x06,
	Alarm_HomeHealth_Unknown_Event = 0xFE
};

enum Alarm_Siren {
	Alarm_Siren_Clear = 0x00,
	Alarm_Siren_Active = 0x01,
	Alarm_Siren_Unknown_Event = 0xFE,
};

enum Alarm_WaterValve {
	Alarm_WaterValve_Clear = 0x00,
	Alarm_WaterValve_Valve_Operation = 0x01,
	Alarm_WaterValve_Master_Valve_Operation = 0x02,
	Alarm_WaterValve_Valve_Short_Circuit = 0x03,
	Alarm_WaterValve_Master_Valve_Short_Circuit = 0x04,
	Alarm_WaterValve_Valve_Current_Alarm = 0x05,
	Alarm_WaterValve_Master_Valve_Current_Alarm = 0x06,
	Alarm_WaterValve_Unknown_Event = 0xFE
};

enum Alarm_Weather {
	Alarm_Weather_Clear = 0x00,
	Alarm_Weather_Rain = 0x01,
	Alarm_Weather_Moisture = 0x02,
	Alarm_Weather_Unknown_Event = 0xFE
};

enum Alarm_Irrigation {
	Alarm_Irrigation_Clear = 0x00,
	Alarm_Irrigation_Schedule_Started = 0x01,
	Alarm_Irrigation_Schedule_Finished = 0x02,
	Alarm_Irrigation_Valve_Table_Run_Started = 0x03,
	Alarm_Irrigation_Valve_Table_Run_Finished = 0x04,
	Alarm_Irrigation_Device_Not_Configured = 0x05,
	Alarm_Irrigation_Unknown_Event = 0xFE
};

enum Alarm_Gas {
	Alarm_Gas_Clear = 0x00,
	Alarm_Gas_Combustable_Gas_Detected = 0x01,
	Alarm_Gas_Combustable_Gas_Detected_Unknown_Location = 0x02,
	Alarm_Gas_Toxic_Gas_Detected = 0x03,
	Alarm_Gas_Toxic_Gas_Detected_Unknown_Location = 0x04,
	Alarm_Gas_Alarm_Test = 0x05,
	Alarm_Gas_Replacement_Required_Unspecified_Reason = 0x06,
	Alarm_Gas_Unknown_Event = 0xFE
};


static std::map<uint8, std::map<uint8, std::string> > c_Alarm_Events;



void SetupAlarmEventDescriptions() {
	c_Alarm_Events[Alarm_Smoke][Alarm_Smoke_Detected_Location] = 						"Smoke Detected";
	c_Alarm_Events[Alarm_Smoke][Alarm_Smoke_Detected_UnknownLocation] =		 			"Smoke Detected";
	c_Alarm_Events[Alarm_Smoke][Alarm_Smoke_Unknown_Event] = 							"Smoke Detected - Unknown";

	c_Alarm_Events[Alarm_CarbonMonoxide][Alarm_CO_Detected_Location] = 					"Carbon Monoxide Detected";
	c_Alarm_Events[Alarm_CarbonMonoxide][Alarm_CO_Detected_UnknownLocation] = 			"Carbon Monoxide Detected";
	c_Alarm_Events[Alarm_CarbonMonoxide][Alarm_CO_Unknown_Event] = 						"Carbon Monoxide Detected - Unknown";

	c_Alarm_Events[Alarm_CarbonDioxide][Alarm_CO2_Detected_Location] = 					"Carbon Dioxide Detected";
	c_Alarm_Events[Alarm_CarbonDioxide][Alarm_CO2_Detected_UnknownLocation] =			"Carbon Dioxide Detected";
	c_Alarm_Events[Alarm_CarbonDioxide][Alarm_CO2_Unknown_Event] = 						"Carbon Dioxide Detected - Unknown";

	c_Alarm_Events[Alarm_Heat][Alarm_Heat_OverHeat_Location] = 							"OverHeat";
	c_Alarm_Events[Alarm_Heat][Alarm_Heat_OverHeat_UnknownLocation] = 					"OverHeat";
	c_Alarm_Events[Alarm_Heat][Alarm_Heat_RapidTempRise_Location] = 					"Temperature Rise";
	c_Alarm_Events[Alarm_Heat][Alarm_Heat_RapidTempRise_UnknownLocation] = 				"Temperature Rise";
	c_Alarm_Events[Alarm_Heat][Alarm_Heat_UnderHeat_Location] = 						"UnderHeat";
	c_Alarm_Events[Alarm_Heat][Alarm_Heat_UnderHeat_UnknownLocation] = 					"UnderHeat";
	c_Alarm_Events[Alarm_Heat][Alarm_Heat_Unknown_Event] = 								"Heat Alarm - Unknown";

	c_Alarm_Events[Alarm_Flood][Alarm_Flood_Leak_Location] = 							"Water Leak";
	c_Alarm_Events[Alarm_Flood][Alarm_Flood_Leak_UnknownLocation] = 					"Water Leak";
	c_Alarm_Events[Alarm_Flood][Alarm_Flood_Drop_Location] = 							"Water Level";
	c_Alarm_Events[Alarm_Flood][Alarm_Flood_Drop_UnknownLocation] = 					"Water Level";
	c_Alarm_Events[Alarm_Flood][Alarm_Flood_Unknown_Event] = 							"Water Alarm - Unknown";

	c_Alarm_Events[Alarm_Access_Control][Alarm_Access_Control_Manual_Lock] = 			"Access Control - Manual Lock";
	c_Alarm_Events[Alarm_Access_Control][Alarm_Access_Control_Manual_Unlock] = 			"Access Control - Manual Unlock";
	c_Alarm_Events[Alarm_Access_Control][Alarm_Access_Control_RF_Lock] = 				"Access Control - RF Lock";
	c_Alarm_Events[Alarm_Access_Control][Alarm_Access_Control_RF_Unlock] = 				"Access Control - RF Unlock";
	c_Alarm_Events[Alarm_Access_Control][Alarm_Access_Control_Keypad_Lock] = 			"Access Control - KeyPad Lock";
	c_Alarm_Events[Alarm_Access_Control][Alarm_Access_Control_Keypad_Unlock] = 			"Access Control - KeyPad Unlock";
	c_Alarm_Events[Alarm_Access_Control][Alarm_Access_Control_Unknown_Event] = 			"Access Control - Unknown";

	c_Alarm_Events[Alarm_HomeSecurity][Alarm_HomeSecurity_Intrusion_Location] = 					"Burglar Intrusion";
	c_Alarm_Events[Alarm_HomeSecurity][Alarm_HomeSecurity_Intrusion_UnknownLocation] = 			"Burglar Intrusion";
	c_Alarm_Events[Alarm_HomeSecurity][Alarm_HomeSecurity_Intrusion_Tamper_CoverRemoved] =		"Burglar Tamper - Cover Removed";
	c_Alarm_Events[Alarm_HomeSecurity][Alarm_HomeSecurity_Intrusion_Tamer_InvalidCode] = 			"Burglar Tamper - Invalid Code";
	c_Alarm_Events[Alarm_HomeSecurity][Alarm_HomeSecurity_GlassBreakage_Location] = 				"Glass Breakage";
	c_Alarm_Events[Alarm_HomeSecurity][Alarm_HomeSecurity_GlassBreakage_UnknownLocation] =		"Glass Breakage";
	c_Alarm_Events[Alarm_HomeSecurity][Alarm_HomeSecurity_Unknown_Event] = 						"Burglar - Unknown";

	c_Alarm_Events[Alarm_Power_Management][Alarm_Power_Management_PowerApplied] = 		"Power Applied";
	c_Alarm_Events[Alarm_Power_Management][Alarm_Power_Management_AC_PowerLost] = 		"AC Power Lost";
	c_Alarm_Events[Alarm_Power_Management][Alarm_Power_Management_AC_PowerRestored] =	"AC Power Restored";
	c_Alarm_Events[Alarm_Power_Management][Alarm_Power_Management_Surge] = 				"Power Surge";
	c_Alarm_Events[Alarm_Power_Management][Alarm_Power_Management_Brownout] = 			"Power Brownout";
	c_Alarm_Events[Alarm_Power_Management][Alarm_Power_Management_Unknown_Event] = 		"Power - Unknown";

	c_Alarm_Events[Alarm_System][Alarm_System_Hardware_Failure] = 						"Hardware Failure";
	c_Alarm_Events[Alarm_System][Alarm_System_Software_Failure] = 						"Software Failure";
	c_Alarm_Events[Alarm_System][Alarm_System_Unknown_Event] = 							"System - Unknown";

	c_Alarm_Events[Alarm_Emergency][Alarm_Emergency_Police] = 							"Emergency - Police";
	c_Alarm_Events[Alarm_Emergency][Alarm_Emergency_Fire] = 							"Emergency - Fire";
	c_Alarm_Events[Alarm_Emergency][Alarm_Emergency_Medical] = 							"Emergency - Medical";
	c_Alarm_Events[Alarm_Emergency][Alarm_Emergency_Unknown_Event] = 					"Emergency - Unknown";

	c_Alarm_Events[Alarm_Clock][Alarm_Clock_Wakeup] = 									"Alarm Clock - WakeUp";

}



//-----------------------------------------------------------------------------
// <WakeUp::WakeUp>
// Constructor
//-----------------------------------------------------------------------------
Alarm::Alarm
(
		uint32 const _homeId,
		uint8 const _nodeId
):
CommandClass( _homeId, _nodeId )
{
	if (c_Alarm_Events.size() == 0)
		SetupAlarmEventDescriptions();

	SetStaticRequest( StaticRequest_Values );
}


//-----------------------------------------------------------------------------
// <Alarm::RequestState>
// Request current state from the device
//-----------------------------------------------------------------------------
bool Alarm::RequestState
(
		uint32 const _requestFlags,
		uint8 const _instance,
		Driver::MsgQueue const _queue
)
{
	if( ( _requestFlags & RequestFlag_Static ) && HasStaticRequest( StaticRequest_Values ) )
	{
		if( GetVersion() > 1 )
		{
			// Request the supported alarm types
			Msg* msg = new Msg( "AlarmCmd_SupportedGet", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
			msg->SetInstance( this, _instance );
			msg->Append( GetNodeId() );
			msg->Append( 2 );
			msg->Append( GetCommandClassId() );
			msg->Append( AlarmCmd_SupportedGet );
			msg->Append( GetDriver()->GetTransmitOptions() );
			GetDriver()->SendMsg( msg, _queue );
			return true;
		}
		else
		{
			/* create version 1 ValueID's */
			if( Node* node = GetNodeUnsafe() )
			{
				node->CreateValueByte( ValueID::ValueGenre_User, GetCommandClassId(), _instance, AlarmIndex_Type, "Alarm Type", "", true, false, 0, 0 );
				node->CreateValueByte( ValueID::ValueGenre_User, GetCommandClassId(), _instance, AlarmIndex_Level, "Alarm Level", "", true, false, 0, 0 );
			}
		}

	}

	if( _requestFlags & RequestFlag_Dynamic )
	{
		return RequestValue( _requestFlags, 0, _instance, _queue );
	}

	return false;
}

//-----------------------------------------------------------------------------
// <Alarm::RequestValue>
// Request current value from the device
//-----------------------------------------------------------------------------
bool Alarm::RequestValue
(
		uint32 const _requestFlags,
		uint16 const _dummy1,	// = 0 (not used)
		uint8 const _instance,
		Driver::MsgQueue const _queue
)
{
	if( IsGetSupported() )
	{
		if( GetVersion() == 1 )
		{
			Msg* msg = new Msg( "AlarmCmd_Get", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
			msg->SetInstance( this, _instance );
			msg->Append( GetNodeId() );
			msg->Append( 2 );
			msg->Append( GetCommandClassId() );
			msg->Append( AlarmCmd_Get );
			msg->Append( GetDriver()->GetTransmitOptions() );
			GetDriver()->SendMsg( msg, _queue );
			return true;
		}
		else
		{
			bool res = false;
			for( uint8 i = 0; i < Alarm_Count; i++ )
			{
				if( Value* value = GetValue( _instance, i + AlarmIndex_Count ) ) {
					value->Release();
					Msg* msg = new Msg( "AlarmCmd_Get", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
					msg->SetInstance( this, _instance );
					msg->Append( GetNodeId() );
					msg->Append( GetVersion() == 2 ? 4 : 5);
					msg->Append( GetCommandClassId() );
					msg->Append( AlarmCmd_Get );
					msg->Append( 0x00); // ? proprietary alarm ?
					msg->Append( i );
					if( GetVersion() > 2 )
						msg->Append(0x01); //get first event of type.
					msg->Append( GetDriver()->GetTransmitOptions() );
					GetDriver()->SendMsg( msg, _queue );
					res = true;
				}
			}
			return res;
		}
	} else {
		Log::Write(  LogLevel_Info, GetNodeId(), "AlarmCmd_Get Not Supported on this node");
	}
	return false;
}

//-----------------------------------------------------------------------------
// <Alarm::HandleMsg>
// Handle a message from the Z-Wave network
//-----------------------------------------------------------------------------
bool Alarm::HandleMsg
(
		uint8 const* _data,
		uint32 const _length,
		uint32 const _instance	// = 1
)
{
	if (AlarmCmd_Report == (AlarmCmd)_data[0])
	{
		// We have received a report from the Z-Wave device
		if( GetVersion() == 1 )
		{
			Log::Write( LogLevel_Info, GetNodeId(), "Received Alarm report: type=%d, level=%d", _data[1], _data[2] );

			ValueByte* value;
			if( (value = static_cast<ValueByte*>( GetValue( _instance, AlarmIndex_Type ) )) )
			{
				value->OnValueRefreshed( _data[1] );
				value->Release();
			}
			// For device on version 1 the level could have different value. This level value correspond to a list of alarm type.
			if ( Value* value = GetValue( _instance, AlarmIndex_Level ) )
			{
				switch ( value->GetID().GetType() )
				{
				case ValueID::ValueType_Byte:
				{
					ValueByte* valueByte = static_cast<ValueByte*>( value );
					valueByte->OnValueRefreshed( _data[2] );
					break;
				}
				case ValueID::ValueType_List:
				{
					ValueList* valueList = static_cast<ValueList*>( value );
					valueList->OnValueRefreshed( _data[2] );
					break;
				}
				default:
				{
					Log::Write( LogLevel_Info, GetNodeId(), "Invalid type (%d) for Alarm Level %d", value->GetID().GetType(), _data[2] );
				}
				}
				value->Release();
			}
		}
		/* version 2 */
		else if(( GetVersion() > 1 ) && ( _length >= 7  ))
		{
			// With Version=2, the data has more detailed information about the alarm

			string alarm_type =  ( _data[5] < Alarm_Count ) ? c_alarmTypeName[_data[5]] : "Unknown type";
			string alarm_event = ( c_Alarm_Events[_data[5]][_data[6]].empty() != true ) ? c_Alarm_Events[_data[5]][_data[6]] : "Unknown Event";

			Log::Write( LogLevel_Info, GetNodeId(), "Received Alarm report: sensorSrcID=%d, type:%s event:%s, status=%s",
					_data[3], alarm_type.c_str(), alarm_event.c_str(), _data[4] == 0x00 ? "false" : "true" );

			{
				ValueByte* value;

				if( (value = static_cast<ValueByte*>( GetValue( _instance, AlarmIndex_SourceNodeId ) )) )
				{
					value->OnValueRefreshed( _data[3] );
					value->Release();
				}
			}
			{
				ValueList *valuel;

				if( (valuel = static_cast<ValueList*>( GetValue( _instance, _data[5]+AlarmIndex_Count ) )) )
				{
					/* if status is 0, then the alarm is inactive */
					if (_data[4] == 0x00)
						valuel->OnValueRefreshed( 0 );
					else
						valuel->OnValueRefreshed( _data[6] );
					valuel->Release();
				}
			}
			/* if the length byte is greater than 1, then there are params */
			if (_data[7] > 0) {
				/* first, delete our old Temp ValueID's */
				for (multimap<uint8, uint8>::iterator it = m_TempValueIDs.begin(); it != m_TempValueIDs.end(); it++) {
					/* first is index, second is instance*/
					if (it->second == _instance) {
						if( Node* node = GetNodeUnsafe() )
						{
							node->RemoveValue(GetCommandClassId(), _instance, it->first);
							m_TempValueIDs.erase(it);
						}
					}
				}

				/* figure out what type of extra data we have */
				switch (_data[5]) {
					/* these alarm types have params */
					case Alarm_Smoke:
						switch(_data[6]) {
							case Alarm_Smoke_Detected_Location:
								string location = ExtractString(&_data[8], _data[7]);
								if( Node* node = GetNodeUnsafe() )
								{
									node->CreateValueString( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (AlarmIndex_Params+m_TempValueIDs.size()), "Location", "", true, false, location, 0 );
									m_TempValueIDs.insert(std::pair<uint8, uint8>(AlarmIndex_Params+m_TempValueIDs.size(), _instance));
								}
							break;
						};
						break;
					case Alarm_CarbonMonoxide:
						switch(_data[6]) {
							case Alarm_CO_Detected_Location:
								string location = ExtractString(&_data[8], _data[7]);
								if( Node* node = GetNodeUnsafe() )
								{
									node->CreateValueString( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (AlarmIndex_Params+m_TempValueIDs.size()), "Location", "", true, false, location, 0 );
									m_TempValueIDs.insert(std::pair<uint8, uint8>(AlarmIndex_Params+m_TempValueIDs.size(), _instance));
								}
							break;
						};
						break;
					case Alarm_CarbonDioxide:
						switch(_data[6]) {
							case Alarm_CO2_Detected_Location:
								string location = ExtractString(&_data[8], _data[7]);
								if( Node* node = GetNodeUnsafe() )
								{
									node->CreateValueString( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (AlarmIndex_Params+m_TempValueIDs.size()), "Location", "", true, false, location, 0 );
									m_TempValueIDs.insert(std::pair<uint8, uint8>(AlarmIndex_Params+m_TempValueIDs.size(), _instance));
								}
							break;
						};
						break;
					case Alarm_Heat:
						switch(_data[6]) {
							case Alarm_Heat_OverHeat_Location:
							case Alarm_Heat_RapidTempRise_Location:
							case Alarm_Heat_UnderHeat_Location:
								string location = ExtractString(&_data[8], _data[7]);
								if( Node* node = GetNodeUnsafe() )
								{
									node->CreateValueString( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (AlarmIndex_Params+m_TempValueIDs.size()), "Location", "", true, false, location, 0 );
									m_TempValueIDs.insert(std::pair<uint8, uint8>(AlarmIndex_Params+m_TempValueIDs.size(), _instance));
								}
							break;
						};
						break;
					case Alarm_Flood:
						switch(_data[6]) {
							case Alarm_Flood_Leak_Location:
							case Alarm_Flood_Leak_UnknownLocation:
							case Alarm_Flood_Drop_Location:
								string location = ExtractString(&_data[8], _data[7]);
								if( Node* node = GetNodeUnsafe() )
								{
									node->CreateValueString( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (AlarmIndex_Params+m_TempValueIDs.size()), "Location", "", true, false, location, 0 );
									m_TempValueIDs.insert(std::pair<uint8, uint8>(AlarmIndex_Params+m_TempValueIDs.size(), _instance));
								}
							break;
						};
						break;
					case Alarm_Access_Control:
						switch(_data[6]) {
							case Alarm_Access_Control_Keypad_Lock:
							case Alarm_Access_Control_Keypad_Unlock:
								/* create version 1 ValueID's */
								if( Node* node = GetNodeUnsafe() )
								{
									node->CreateValueByte( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (AlarmIndex_Params+m_TempValueIDs.size()), "UserCode", "", true, false, _data[8], 0 );
									m_TempValueIDs.insert(std::pair<uint8, uint8>(AlarmIndex_Params+m_TempValueIDs.size(), _instance));
								}
							break;
						};
						break;
					case Alarm_HomeSecurity:
						switch(_data[6]) {
							case Alarm_HomeSecurity_Intrusion_Location:
							case Alarm_HomeSecurity_GlassBreakage_Location:
								string location = ExtractString(&_data[8], _data[7]);
								if( Node* node = GetNodeUnsafe() )
								{
									node->CreateValueString( ValueID::ValueGenre_User, GetCommandClassId(), _instance, (AlarmIndex_Params+m_TempValueIDs.size()), "Location", "", true, false, location, 0 );
									m_TempValueIDs.insert(std::pair<uint8, uint8>(AlarmIndex_Params+m_TempValueIDs.size(), _instance));
								}
							break;
						};
						break;
				}


			}


		}

		return true;
	}

	if( AlarmCmd_SupportedReport == (AlarmCmd)_data[0] )
	{
		if( Node* node = GetNodeUnsafe() )
		{
			// We have received the supported alarm types from the Z-Wave device
			Log::Write( LogLevel_Info, GetNodeId(), "Received supported alarm types" );

			node->CreateValueByte( ValueID::ValueGenre_User, GetCommandClassId(), _instance, AlarmIndex_SourceNodeId, "SourceNodeId", "", true, false, 0, 0 );

			// Parse the data for the supported alarm types
			uint8 numBytes = _data[1];
			for( uint32 i=0; i<numBytes; ++i )
			{
				for( int32 bit=0; bit<8; ++bit )
				{
					if( ( _data[i+2] & (1<<bit) ) != 0 )
					{
						int32 index = (int32)(i<<3) + bit;
						if( index < Alarm_Count )
						{
							vector<ValueList::Item> _items;
							ValueList::Item item;
							item.m_value = 0;
							item.m_label = "Not Active";
							_items.push_back( item );

							for (map<uint8, std::string>::iterator it = c_Alarm_Events[index].begin(); it != c_Alarm_Events[index].end(); it++) {
								ValueList::Item item;
								item.m_value = it->first;
								item.m_label = it->second;
								_items.push_back( item );
							}
							node->CreateValueList( ValueID::ValueGenre_User, GetCommandClassId(), _instance, index+AlarmIndex_Count, c_alarmTypeName[index], "", false, false, _items.size(), _items, 0, 0 );
							Log::Write( LogLevel_Info, GetNodeId(), "    Added alarm type: %s", c_alarmTypeName[index] );
						} else {
							Log::Write( LogLevel_Info, GetNodeId(), "    Unknown alarm type: %d", index );
						}
					}
				}
			}
		}

		ClearStaticRequest( StaticRequest_Values );
		return true;
	}

	return false;
}



