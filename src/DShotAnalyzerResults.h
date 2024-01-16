#ifndef DSHOT_ANALYZER_RESULTS
#define DSHOT_ANALYZER_RESULTS

#include <AnalyzerResults.h>


#define ERROR_FLAG_CRC 		( 1 << 0 )
#define ERROR_FLAG_FRAMING 	( 1 << 1 )
#define ERROR_FLAG_GRC		( 1 << 2 )

enum FrameType {
	VALUE,
	COMMAND,
	TELEMETRY,
};

enum EDTTypes {
	RPM = 0,
	TEMPERATURE = 0x02,
	VOLTAGE = 0x04,
	CURRENT = 0x06,
	DEBUG1 = 0x08,
	DEBUG2 = 0x0A,
	DEBUG3 = 0x0C,
	STATE  = 0x0E,
};



class DShotAnalyzer;
class DShotAnalyzerSettings;

class DShotAnalyzerResults : public AnalyzerResults
{
public:
	DShotAnalyzerResults( DShotAnalyzer* analyzer, DShotAnalyzerSettings* settings );
	virtual ~DShotAnalyzerResults();

	virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
	virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base );
	virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
	virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

	U64 rpm_from_period(U64);

protected: //functions

protected:  //vars
	DShotAnalyzerSettings* mSettings;
	DShotAnalyzer* mAnalyzer;
};

#endif //DSHOT_ANALYZER_RESULTS
