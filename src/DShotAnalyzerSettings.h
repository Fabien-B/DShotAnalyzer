#pragma once

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class DShotAnalyzerSettings : public AnalyzerSettings
{
public:
	DShotAnalyzerSettings();
	virtual ~DShotAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	double mBitRate;
	bool mBidir;

protected:
	std::unique_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mBitRateInterface;
	std::unique_ptr< AnalyzerSettingInterfaceBool >	mBidirInterface;
};
