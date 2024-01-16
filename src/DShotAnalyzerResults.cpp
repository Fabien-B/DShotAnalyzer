#include "DShotAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "DShotAnalyzer.h"
#include "DShotAnalyzerSettings.h"
#include <iostream>
#include <fstream>



const char* COMMANDS[] = {
	"DSHOT_CMD_MOTOR_STOP",
	"DSHOT_CMD_BEEP1",
	"DSHOT_CMD_BEEP2",
	"DSHOT_CMD_BEEP3",
	"DSHOT_CMD_BEEP4",
	"DSHOT_CMD_BEEP5",
	"DSHOT_CMD_ESC_INFO",
	"DSHOT_CMD_SPIN_DIRECTION_1",
	"DSHOT_CMD_SPIN_DIRECTION_2",
	"DSHOT_CMD_3D_MODE_OFF",
	"DSHOT_CMD_3D_MODE_ON",
	"DSHOT_CMD_SETTINGS_REQUEST",
	"DSHOT_CMD_SAVE_SETTINGS",
	"DSHOT_EXTENDED_TELEMETRY_ENABLE",
	"DSHOT_EXTENDED_TELEMETRY_DISABLE",
	"-",
	"-",
	"-",
	"-",
	"-",
	"DSHOT_CMD_SPIN_DIRECTION_NORMAL",
	"DSHOT_CMD_SPIN_DIRECTION_REVERSED",
	"DSHOT_CMD_LED0_ON",
	"DSHOT_CMD_LED1_ON",
	"DSHOT_CMD_LED2_ON",
	"DSHOT_CMD_LED3_ON",
	"DSHOT_CMD_LED0_OFF",
	"DSHOT_CMD_LED1_OFF",
	"DSHOT_CMD_LED2_OFF",
	"DSHOT_CMD_LED3_OFF",
	"Audio_Stream mode on/Off",
	"Silent Mode on/Off",
	"DSHOT_CMD_SIGNAL_LINE_TELEMETRY_DISABLE",
	"DSHOT_CMD_SIGNAL_LINE_TELEMETRY_ENABLE",
	"DSHOT_CMD_SIGNAL_LINE_CONTINUOUS_ERPM_TELEMETRY",
	"DSHOT_CMD_SIGNAL_LINE_CONTINUOUS_ERPM_PERIOD_TELEMETRY",
	"-",
	"-",
	"-",
	"-",
	"-",
	"-",
	"DSHOT_CMD_SIGNAL_LINE_TEMPERATURE_TELEMETRY",
	"DSHOT_CMD_SIGNAL_LINE_VOLTAGE_TELEMETRY",
	"DSHOT_CMD_SIGNAL_LINE_CURRENT_TELEMETRY",
	"DSHOT_CMD_SIGNAL_LINE_CONSUMPTION_TELEMETRY",
	"DSHOT_CMD_SIGNAL_LINE_ERPM_TELEMETRY",
	"DSHOT_CMD_SIGNAL_LINE_ERPM_PERIOD_TELEMETRY",
};




DShotAnalyzerResults::DShotAnalyzerResults( DShotAnalyzer* analyzer, DShotAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

DShotAnalyzerResults::~DShotAnalyzerResults()
{
}

void DShotAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );

	if(frame.mFlags) {
		if(frame.mFlags & ERROR_FLAG_FRAMING) {
			AddResultString( "FRAMING_ERROR" );
		}
		else if(frame.mFlags & ERROR_FLAG_GRC) {
			AddResultString( "GRC_ERROR" );
		}
		else if(frame.mFlags & ERROR_FLAG_CRC) {
			AddResultString( "CRC_ERROR" );
		}
		else {
			AddResultString( "ERROR" );
		}
		
		
	}
	else if(frame.mType == FrameType::VALUE) {
		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );
		AddResultString( "THROTTLE: ", number_str );
	}
	else if(frame.mType == FrameType::COMMAND) {
		if(frame.mData1 < 48) {
			AddResultString( "CMD: ", COMMANDS[frame.mData1] );
		}
	}
	else if(frame.mType == FrameType::TELEMETRY) {
		char number_str[128];
		switch (frame.mData2)
		{
		case EDTTypes::RPM:
			AnalyzerHelpers::GetNumberString( rpm_from_period(frame.mData1), display_base, 16, number_str, 128 );
			AddResultString( "RPM: ", number_str );
			break;
		case EDTTypes::TEMPERATURE:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString( "TEMPERATURE: ", number_str );
			break;
		case EDTTypes::VOLTAGE:
			AnalyzerHelpers::GetNumberString( 25*frame.mData1, display_base, 16, number_str, 128 );
			AddResultString( "TEMPERATURE: ", number_str );
			break;
		case EDTTypes::CURRENT:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString( "TEMPERATURE: ", number_str );
			break;
		case EDTTypes::STATE:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddResultString( "TEMPERATURE: ", number_str );
			break;
		
		default:
			AddResultString( "OTHER" );
			break;
		}
		
	}
}

void DShotAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time[s],type,Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );


		if(frame.mFlags) {
			file_stream << time_str << ",";
			if(frame.mFlags & ERROR_FLAG_CRC) {
				file_stream << "CRC";
			}
			if(frame.mFlags & ERROR_FLAG_FRAMING) {
				file_stream << "FRAMING";
			}
			if(frame.mFlags & ERROR_FLAG_GRC) {
				file_stream << "GRC";
			}
			file_stream << "_ERROR" << "," << std::endl;
		}
		else if(frame.mType == FrameType::VALUE) {
			char number_str[128];
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );
			file_stream << time_str << "," << "THROTTLE" << "," << number_str << std::endl;
		}
		else if(frame.mType == FrameType::COMMAND) {
			if(frame.mData1 < 48) {
				file_stream << time_str << "," << "CMD" << "," << COMMANDS[frame.mData1] << std::endl;
			}
		}
		else if(frame.mType == FrameType::TELEMETRY) {
			char number_str[128];
			switch (frame.mData2)
			{
			case EDTTypes::RPM:
				AnalyzerHelpers::GetNumberString( rpm_from_period(frame.mData1), display_base, 16, number_str, 128 );
				file_stream << time_str << "," << "RPM" << "," << number_str;
				break;
			case EDTTypes::TEMPERATURE:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
				file_stream << time_str << "," << "TEMPERATURE" << "," << number_str;
				break;
			case EDTTypes::VOLTAGE:
				AnalyzerHelpers::GetNumberString( 25*frame.mData1, display_base, 16, number_str, 128 );
				file_stream << time_str << "," << "VOLTAGE" << "," << number_str;
				break;
			case EDTTypes::CURRENT:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
				file_stream << time_str << "," << "CURRENT" << "," << number_str;
				break;
			case EDTTypes::STATE:
				AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
				file_stream << time_str << "," << "STATE" << "," << number_str;
				break;
			
			default:
				file_stream << time_str << "," << "OTHER";
				break;
			}
			
		}

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void DShotAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
	ClearTabularText();
	Frame frame = GetFrame( frame_index );
	if(frame.mFlags) {
		if(frame.mFlags & ERROR_FLAG_CRC) {
			AddTabularText( "CRC_ERROR");
		}
		if(frame.mFlags & ERROR_FLAG_FRAMING) {
			AddTabularText( "FRAMING_ERROR");
		}
		if(frame.mFlags & ERROR_FLAG_GRC) {
			AddResultString( "GRC_ERROR" );
		}
		
	}
	else if(frame.mType == FrameType::VALUE) {
		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );
		AddTabularText( "THROTTLE ", number_str );
	}
	else if(frame.mType == FrameType::COMMAND) {
		if(frame.mData1 < 48) {
			AddTabularText( "CMD ", COMMANDS[frame.mData1] );
		}
		else {
			char number_str[128];
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );
			AddTabularText( "CMD! ", number_str, " (error > 47)" );
		}
	}
	else if(frame.mType == FrameType::TELEMETRY) {
		char number_str[128];
		switch (frame.mData2)
		{
		case EDTTypes::RPM:
			{
				AnalyzerHelpers::GetNumberString( rpm_from_period(frame.mData1), display_base, 64, number_str, 128 );
				AddTabularText( "RPM ",number_str );
			}
			break;
		case EDTTypes::TEMPERATURE:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 16, number_str, 128 );
			AddTabularText( "TEMPERATURE ", number_str );
			break;
		case EDTTypes::VOLTAGE:
			AnalyzerHelpers::GetNumberString( 25*frame.mData1, display_base, 16, number_str, 128 );
			AddTabularText( "VOLTAGE ", number_str );
			break;
		case EDTTypes::CURRENT:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddTabularText( "CURRENT ", number_str );
			break;
		case EDTTypes::STATE:
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
			AddTabularText( "STATE ", number_str );
			break;
		default:
			AddTabularText( "OTHER" );
			break;
		}
		
	}
	
#endif
}

void DShotAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	//not supported

}

void DShotAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	//not supported
}

U64 DShotAnalyzerResults::rpm_from_period(U64 period) {
	U64 erpm = 60e6/period;
	if(period == 65408) {
		erpm = 0;
	}
	U64 rpm = erpm / mSettings->mPolepairs;
	return rpm;
}
