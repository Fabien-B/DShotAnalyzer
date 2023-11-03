#ifndef DSHOT_ANALYZER_H
#define DSHOT_ANALYZER_H

#include <Analyzer.h>
#include "DShotAnalyzerResults.h"
#include "DShotSimulationDataGenerator.h"

class DShotAnalyzerSettings;
class ANALYZER_EXPORT DShotAnalyzer : public Analyzer2
{
public:
	DShotAnalyzer();
	virtual ~DShotAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::unique_ptr< DShotAnalyzerSettings > mSettings;
	std::unique_ptr< DShotAnalyzerResults > mResults;
	AnalyzerChannelData* mDShot;

	DShotSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitialized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //DSHOT_ANALYZER_H
