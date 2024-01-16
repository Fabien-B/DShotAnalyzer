#include "DShotAnalyzer.h"
#include "DShotAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

U8 GRC_ENCODING[16] = {
	0x19, 0x1B, 0x12, 0x13, 0x1D, 0x15, 0x16, 0x17, 0x1A, 0x09, 0x0A, 0x0B, 0x1E, 0x0D, 0x0E, 0x0F
};

DShotAnalyzer::DShotAnalyzer()
:	Analyzer2(),  
	mSettings( new DShotAnalyzerSettings() ),
	mSimulationInitialized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

DShotAnalyzer::~DShotAnalyzer()
{
	KillThread();
}

void DShotAnalyzer::SetupResults()
{
	mResults.reset( new DShotAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void DShotAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();

	mDShot = GetAnalyzerChannelData( mSettings->mInputChannel );
	U32 samples_per_bit = mSampleRateHz / (mSettings->mBitRate * 1000);

	BitState active_state = mSettings->mBidir == DShotAnalyzerSettings::Telemetry::NONE ? BIT_HIGH : BIT_LOW;

	

	while( mDShot->GetBitState() == active_state )
	{
		// in the middle of a frame, go out of the frame
		mDShot->Advance(16 * samples_per_bit);
	}


	for( ; ; )
	{
		U8 flags = 0;
		U16 data = 0;
		U16 mask = 1<<15;

		U64 starting_sample = 0;
		U64 frame_end_sample = 0;

		for(int i=0; i<16; i++) {
			// go to the rising edge at the frame start
			mDShot->AdvanceToNextEdge();
			U64 bit_start_sample = mDShot->GetSampleNumber();
			

			if(i==0) {
				starting_sample = bit_start_sample;
				mResults->AddMarker( mDShot->GetSampleNumber(), AnalyzerResults::Start, mSettings->mInputChannel);
			} else {
				if(mSettings->mBidir == DShotAnalyzerSettings::Telemetry::NONE) {
					mResults->AddMarker( mDShot->GetSampleNumber(), AnalyzerResults::UpArrow, mSettings->mInputChannel);
				} else {
					mResults->AddMarker( mDShot->GetSampleNumber(), AnalyzerResults::DownArrow, mSettings->mInputChannel);
				}
				
			}
			


			// go to the falling edge at the middle of the bit
			mDShot->AdvanceToNextEdge();
			U64 bit_change_sample = mDShot->GetSampleNumber();
			double duty = (bit_change_sample - bit_start_sample) / static_cast<double>(samples_per_bit);

			// put a dot at the middle of the bit
			U64 dot_sample = bit_start_sample + samples_per_bit/2;
			
			
			if(i==11) {
				// mark telemetry bit
				mResults->AddMarker( dot_sample, AnalyzerResults::Square, mSettings->mInputChannel);
			} else {
				// normal bit
				mResults->AddMarker( dot_sample, AnalyzerResults::Dot, mSettings->mInputChannel);
			}

			if(duty > 0.5) {
				data |= mask;
			}
			mask = mask >> 1;


			// always estimate the end of the frame to account for small bitrate deviation
			frame_end_sample = bit_start_sample + (16-i) * samples_per_bit;
			if(i<15) {
				U64 next_bit_sample = mDShot->GetSampleOfNextEdge();
				if(next_bit_sample > frame_end_sample) {
					frame_end_sample = mDShot->GetSampleNumber();
					flags |= ERROR_FLAG_FRAMING;
					flags |= DISPLAY_AS_ERROR_FLAG;
					mResults->AddMarker( frame_end_sample, AnalyzerResults::ErrorX, mSettings->mInputChannel);
					break;
				}
			}
		}

		U16 crc_calc;
        U16 thte = data >> 4;
		if(mSettings->mBidir == DShotAnalyzerSettings::Telemetry::NONE) {
			crc_calc =  (thte ^ (thte >> 4) ^ (thte >> 8)) & 0x0F;
		} else {
			crc_calc =(~(thte ^ (thte >> 4) ^ (thte >> 8))) & 0x0F;
		}

		U16 crc_received = data & 0x0F;

		// Put CRC_ERROR flag only if there is no framing error
		if(crc_calc != crc_received &&
			!(flags & ERROR_FLAG_FRAMING)) {
			flags |= ERROR_FLAG_CRC;
			flags |= DISPLAY_AS_ERROR_FLAG;
		}

		U16 value = data >> 5;
		U8 telemetry_requested = (data >> 4) & 0x01;

		//we have a U16 to save. 
		Frame frame;
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = frame_end_sample;
		frame.mFlags = flags;
		frame.mData2 = telemetry_requested;

		if(value < 48) {
			frame.mType = FrameType::COMMAND;
			frame.mData1 = value;
		} else {
			frame.mType = FrameType::VALUE;
			frame.mData1 = value - 47;
		}

		// move to the end of the last bit
		if(mDShot->AdvanceToAbsPosition(frame_end_sample) > 0) {
			frame.mFlags |= DISPLAY_AS_ERROR_FLAG;
			mResults->AddMarker( frame_end_sample, AnalyzerResults::ErrorSquare, mSettings->mInputChannel);
		}

		mResults->AddFrame( frame );
		mResults->CommitResults();
		ReportProgress( frame.mEndingSampleInclusive );



		if(mSettings->mBidir != DShotAnalyzerSettings::Telemetry::NONE) {
				Frame erpm_frame;
				erpm_frame.mType = FrameType::TELEMETRY;
				
				mDShot->AdvanceToNextEdge();
				U64 start = mDShot->GetSampleNumber();
				erpm_frame.mStartingSampleInclusive = start;

				mResults->AddMarker( mDShot->GetSampleNumber(), AnalyzerResults::DownArrow, mSettings->mInputChannel);
				double spb = samples_per_bit / 1.1534025374855825;
				BitState last_bit = BIT_LOW;
				U32 grc = 0;
				U32 mask = 1<<20;
				for(int i=0; i<21; i++) {
					mDShot->AdvanceToAbsPosition(start + spb/2 + i*spb);
					BitState bit = mDShot->GetBitState();
					
					if(bit == last_bit) {
						mResults->AddMarker( mDShot->GetSampleNumber(), AnalyzerResults::Zero, mSettings->mInputChannel);
					} else {
						grc |= mask;
						mResults->AddMarker( mDShot->GetSampleNumber(), AnalyzerResults::One, mSettings->mInputChannel);
					}
					last_bit = bit;
					mask >>= 1;
				}
				mDShot->AdvanceToAbsPosition(start + spb/2 + 21*spb);
				erpm_frame.mEndingSampleInclusive = mDShot->GetSampleNumber();
				//erpm_frame.mFlags = flags;
				U16 data = 0;
				for(int i=0; i<4; i++) {
					U8 grc_nibble = grc & 0x1F;
					for(int j=0; j<16; j++) {
						if(GRC_ENCODING[j] == grc_nibble) {
							data |= j << (4*i);
							break;
						}
						else {
							if(j==15) {
								// grc code not found in the lookup table
								erpm_frame.mFlags |= (DISPLAY_AS_ERROR_FLAG | ERROR_FLAG_GRC);
							}
						}
					}
					grc >>= 5;
				}

				U16 erpm_data = data >> 4;
				crc_calc =  (erpm_data ^ (erpm_data >> 4) ^ (erpm_data >> 8)) & 0x0F;
				U16 crc_received = data & 0x0F;

				// Put CRC_ERROR flag only if there is no framing error
				if(crc_calc != crc_received) {
					flags |= ERROR_FLAG_CRC;
					flags |= DISPLAY_AS_ERROR_FLAG;
				}

				if(mSettings->mBidir == DShotAnalyzerSettings::Telemetry::EXTENDED &&
					(erpm_data & (1<<8)) == 0) {
					// special EDT frame
					U8 packet_type = erpm_data >> 8;
					U8 value = erpm_data & 0xFF;
					erpm_frame.mData1 = value;
					erpm_frame.mData2 = packet_type;
					
				}
				else {
					// normal eRPM telemetry
					U64 rpm = (erpm_data & 0x1FF) << (erpm_data>>9);
					erpm_frame.mData1 = rpm;
					erpm_frame.mData2 = EDTTypes::RPM;
				}
				
				mResults->AddFrame( erpm_frame );
				mResults->CommitResults();
				ReportProgress( erpm_frame.mEndingSampleInclusive );
		}


	}
}

bool DShotAnalyzer::NeedsRerun()
{
	return false;
}

U32 DShotAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitialized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitialized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 DShotAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4000;
}

const char* DShotAnalyzer::GetAnalyzerName() const
{
	return "DShot";
}

const char* GetAnalyzerName()
{
	return "DShot";
}

Analyzer* CreateAnalyzer()
{
	return new DShotAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}