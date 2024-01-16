#pragma once

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class DShotAnalyzerSettings : public AnalyzerSettings
{
public:

	enum Telemetry {
		NONE,
		SIMPLE,
		EXTENDED
	};

	DShotAnalyzerSettings();
	virtual ~DShotAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	double mBitRate;
	enum Telemetry mBidir;
	int mPolepairs;

protected:
	std::unique_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mBitRateInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mBidirInterface;
	std::unique_ptr< AnalyzerSettingInterfaceInteger >	mPolePairsInterface;
};
