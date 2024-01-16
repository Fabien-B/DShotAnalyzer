#include "DShotAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


DShotAnalyzerSettings::DShotAnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mBitRate( 9600 ),
	mBidir(Telemetry::NONE),
	mPolepairs(7)
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "Channel", "DShot" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mBitRateInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/S)",  "Specify the bit rate in bits per second." );
	mBitRateInterface->AddNumber(300, "300", "DShot 300");
	mBitRateInterface->AddNumber(600, "600", "DShot 600");
	mBitRateInterface->SetNumber(mBitRate);

	mBidirInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mBidirInterface->SetTitleAndTooltip("Bidirectionnal", "Bidirectionnal telemetry");
	mBidirInterface->AddNumber(DShotAnalyzerSettings::Telemetry::NONE, "None", "no bidirectionnal");
	mBidirInterface->AddNumber(DShotAnalyzerSettings::Telemetry::SIMPLE, "Bidir", "bidirectionnal DSHOT");
	mBidirInterface->AddNumber(DShotAnalyzerSettings::Telemetry::EXTENDED, "Extended", "Extended DShot Telemetry (EDT)");

	mPolePairsInterface.reset(new AnalyzerSettingInterfaceInteger());
	mPolePairsInterface->SetTitleAndTooltip("Pole pairs number", "number of pole pairs (number of magnets / 2)");


	AddInterface( mInputChannelInterface.get() );
	AddInterface( mBitRateInterface.get() );
	AddInterface( mBidirInterface.get() );
	AddInterface( mPolePairsInterface.get() );


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
	mBidir = static_cast<Telemetry>(mBidirInterface->GetNumber());
	mPolepairs = mPolePairsInterface->GetInteger();

	ClearChannels();
	AddChannel( mInputChannel, "DShot", true );

	return true;
}

void DShotAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mBitRateInterface->SetNumber( mBitRate );
	mBidirInterface->SetNumber( mBidir );
	mPolePairsInterface->SetInteger(mPolepairs);
}

void DShotAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );
	int bidir;
	text_archive >> mInputChannel;
	text_archive >> mBitRate;
	text_archive >> bidir;
	mBidir = static_cast<Telemetry>(bidir);
	text_archive >> mPolepairs;

	ClearChannels();
	AddChannel( mInputChannel, "DShot", true );

	UpdateInterfacesFromSettings();
}

const char* DShotAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
	text_archive << mBitRate;
	text_archive << static_cast<int>(mBidir);
	text_archive << mPolepairs;

	return SetReturnString( text_archive.GetString() );
}
