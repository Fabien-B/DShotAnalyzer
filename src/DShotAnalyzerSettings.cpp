#include "DShotAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


DShotAnalyzerSettings::DShotAnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mBitRate( 9600 ),
	mBidir(false)
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "Channel", "DShot" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mBitRateInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/S)",  "Specify the bit rate in bits per second." );
	mBitRateInterface->AddNumber(300, "300", "DShot 300");
	mBitRateInterface->AddNumber(600, "600", "DShot 600");
	mBitRateInterface->SetNumber(mBitRate);

	mBidirInterface.reset(new AnalyzerSettingInterfaceBool());
	mBidirInterface->SetTitleAndTooltip("Bidirectionnal", "Bidirectionnal telemetry");
	mBidirInterface->SetValue(mBidir);

	AddInterface( mInputChannelInterface.get() );
	AddInterface( mBitRateInterface.get() );
	AddInterface( mBidirInterface.get() );


	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "DShot", false );
}

DShotAnalyzerSettings::~DShotAnalyzerSettings()
{
}

bool DShotAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	mBitRate = mBitRateInterface->GetNumber();
	mBidir = mBidirInterface->GetValue();;

	ClearChannels();
	AddChannel( mInputChannel, "DShot", true );

	return true;
}

void DShotAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mBitRateInterface->SetNumber( mBitRate );
	mBidirInterface->SetValue( mBidir );
}

void DShotAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;
	text_archive >> mBitRate;
	text_archive >> mBidir;

	ClearChannels();
	AddChannel( mInputChannel, "DShot", true );

	UpdateInterfacesFromSettings();
}

const char* DShotAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
	text_archive << mBitRate;
	text_archive << mBidir;

	return SetReturnString( text_archive.GetString() );
}
