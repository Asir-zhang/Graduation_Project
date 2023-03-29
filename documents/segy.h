



#ifndef __SEGY__H__
#define __SEGY__H__
#define __USE_FILE_OFFSET64
#define __USE_LARGEFILE64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "segy_su.h"

/*************************************************************************/

typedef struct{
  long  NoJob;                /* 01  3201-3204 ------------------------------------------------------------------
                                   Job identification number   */
  long  NoLine;               /* 02  3205-3208 ------------------------------------------------------------------
                                   Line number. For 3-D poststack data, this will typically contain the in-linenumber. */
  long  NoReel;               /* 03  3209-3212 ------------------------------------------------------------------
                                   Reel number.*/
  short NumDataTrace;         /* 04  3213-3214 ------------------------------------------------------------------
                                   Number of data traces per ensemble. Mandatory for prestack data.*/
  short NumAuxiTrace;         /* 05  3215-3216 ------------------------------------------------------------------
                                   Number of auxiliary traces per ensemble. Mandatory for prestack data.*/
  short SampleRatioReel;      /* 06  3217-3218 ------------------------------------------------------------------
                                   Sample interval in microseconds (�). Mandatory for all data types.*/
  short SampleRatioField;     /* 07  3219-3220 ------------------------------------------------------------------
                                   Sample interval in microseconds (�) of original field recording.*/
  short SampleNumReel;        /* 08  3221-3222
                                   Number of samples per data trace. Mandatory for all types of data.
                                   Note: The sample interval and number of samples in the Binary File Header
                                   should be for the primary set of seismic data traces in the file.*/
  short SampleNumField;       /* 09  3223-3224
                                   Number of samples per data trace for original field recording.*/
  short DataFormat;           /* 10  3225-3226
                                   Data sample format code. Mandatory for all data.
                                       1 = 4-byte IBM floating-point
                                       2 = 4-byte, two's complement integer
                                       3 = 2-byte, two's complement integer
                                       4 = 4-byte fixed-point with gain (obsolete)
                                       5 = 4-byte IEEE floating-point
                                       6 = Not currently used
                                       7 = Not currently used
                                       8 = 1-byte, two's complement integer*/
  short CDPFold;              /* 11  3227-3228
                                   Ensemble fold  The expected number of data traces per trace ensemble
                                   (e.g. the CMP fold). Highly recommended for all types of data.*/
  short TraceSortCode;        /* 12  3229-3230
                                   Trace sorting code (i.e. type of ensemble):
                                      -1 = Other (should be explained in user Extended Textual File Header stanza
                                       0 = Unknown
                                       1 = As recorded (no sorting)
                                       2 = CDP ensemble
                                       3 = Single fold continuous profile
                                       4 = Horizontally stacked
                                       5 = Common source
                                       6 = Common receiver
                                       7 = Common offset
                                       8 = Common mid-point*/
  short VerticalSumCode;      /* 13  3231-3232
                                   Vertical sum code:
                                       1 = no sum,
                                       2 = two sum,
                                          ....,
                                       N = N sum (N = 1 to 32,767)*/
  short SweepFreqStart;       /* 14   3233-3234
                                   Sweep frequency at start.*/
  short SweepFreqEnd;         /* 15  3235-3236
                                   Sweep frequency at end.*/
  short SweepLength;          /* 16  3237-3238
                                   Sweep length (ms).*/
  short SweepTypeCode;        /* 17   3239-3240
                                   Sweep type code: 1 = linear 2 = parabolic 3 = exponential 4 = other*/
  short SweepTraceNum;        /* 18   3241-3242
                                   Trace number of sweep channel.*/
  short SweepTaperLenStart;   /* 19  3243-3244
                                   Sweep trace taper length in milliseconds at start if tapered
                                   (the taper starts at zero time and is effective for this length).*/
  short SweepTaperLenEnd;     /* 20   3245-3246
                                   Sweep trace taper length in milliseconds at end (the ending taper
                                   starts at sweep length minus the taper length at end).*/
  short SweepTaperType;       /* 21  3247-3248
                                   Taper type:1 = linear; 2 = cos^2; 3 = other*/
  short CorrelDataTrace;      /* 22  3249-3250
                                   Correlated data traces:1 = no; 2 = yes*/
  short BinaryGainRecover;    /* 23  3251-3252
                                   Binary gain recovered:
                                       1 = yes; 2 = no*/
  short AmpRecoverMethod;     /* 24  3253-3254
                                   Amplitude recovery method:
                                       1 = none
                                       2 = spherical divergence
                                       3 = AGC
                                       4 = other*/
  short MeasurementSys;       /* 25  3255-3256
                                   Measurement system: Highly recommended for all types of data. If Location
                                   Data stanzas are included in the file, this entry must agree with the Location
                                   Data stanza. If there is a disagreement, the Location Data stanza is the
                                   controlling authority.
                                       1 = Meters     2 = Feet*/
  short ImpulseSignal;        /* 26  3257-3258
                                    Impulse signal polarity
                                       1 = Increase in pressure or upward geophone case movement gives
                                           negative number on tape.
                                       2 = Increase in pressure or upward geophone case movement gives
                                           positive number on tape.   */
  short VibratoryPolarityCode;/* 27  3259-3260
                                    Vibratory polarity code:  Seismic signal lags pilot signal by:
                                       1 = 337.5�to 22.5�                                       2 = 22.5�to 67.5�                                       3 = 67.5�to 112.5�                                       4 = 112.5�to 157.5�                                       5 = 157.5�to 202.5�                                       6 = 202.5�to 247.5�                                       7 = 247.5�to 292.5�                                       8 = 292.5�to 337.5*/
  int  Reserved1[60];         /* 28  3261-3500*/
  unsigned short Revision;    /* 29  3501-3502
                                   SEG Y Format Revision Number. This is a 16-bit unsigned value with a Q-
                                   point between the first and second bytes. Thus for SEG Y Revision 1.0, as
                                   defined in this document, this will be recorded as 0x0100 . This field is
                                   mandatory for all versions of SEG Y, although a value of zero indicates
                                   traditional SEG Y conforming to the 1975 standard.*/
  short TraceLengthFlag;      /* 30   3503-3504
                                   Fixed length trace flag. A value of one indicates that all traces in this SEG Y
                                   file are guaranteed to have the same sample interval and number of samples,
                                   as specified in Textual File Header bytes 3217-3218 and 3221-3222. A value
                                   of zero indicates that the length of the traces in the file may vary and the
                                   number of samples in bytes 115-116 of the Trace Header must be examined to
                                   determine the actual length of each trace.*/
  short ExtTextNum;          /*  31 3505-3506
                                   Number of 3200-byte Textual Header Extension records following the Binary
                                   Header. A value of zero indicates there are no Textual Header Extension
                                   records (i.e. this file has no Extended Textual Header). A value of -1 indicates
                                   that there are a variable number of Textual Header Extension records and the
                                   end of the Extended Textual Header is denoted by an ((EndText)) stanza in the
                                   final record. A positive value indicates that there are exactly that many Textual
                                   Header Extension records. Note that, although the exact number of Textual
                                   Header Extension records may be a useful piece of information, it will not
                                   always be known at the time the Binary Header is written and it would not be
                                   reasonable to insist that a positive value be recorded here.*/
  char  ReservedChar2[94];    /* 32 3507-3600*/

}SEGY_BFILE_HEAD;


typedef struct{
  long TraceNoLine;           /* 01  001-004
                                   Trace sequence number within line  Numbers continue to increase if the same
                                   line continues across multiple SEG Y files.*/
  long TraceNoReel;           /* 02  005-008
                                   Trace sequence number within SEG Y file  Each file starts with trace sequence one.*/
  long RecordNoField;         /* 03  009-012
                                   Original field record number.*/
  long TraceNoField;          /* 04  013-016
                                   Trace number within the original field record.*/
  long SourceNo;              /* 05  017-020
                                   Energy source point number  Used when more than one record occurs at the
                                   same effective surface location. It is recommended that the new entry defined in
                                   Trace Header bytes 197-202 be used for shotpoint number.*/
  long CDPNo;                 /* 06  021-024
                                   CDP ensemble number*/
  long TraceNoCDP;            /* 07  025-028
                                   Trace number within the CDP ensemble  Each ensemble starts with trace number one.*/
  short TraceCode;            /* 08  029-030
                                   Trace identification code:
                                      -1 = Other
                                       0 = Unknown
                                       1 = Seismic data
                                       2 = Dead
                                       3 = Dummy
                                       4 = Time break
                                       5 = Uphole
                                       6 = Sweep
                                       7 = Timing
                                       8 = Waterbreak
                                       9 = Near-field gun signature
                                      10 = Far-field gun signature
                                      11 = Seismic pressure sensor
                                      12 = Multicomponent seismic sensor - Vertical component
                                      13 = Multicomponent seismic sensor - Cross-line component
                                      14 = Multicomponent seismic sensor - In-line component
                                      15 = Rotated multicomponent seismic sensor - Vertical component
                                      16 = Rotated multicomponent seismic sensor - Transverse component
                                      17 = Rotated multicomponent seismic sensor - Radial component
                                      18 = Vibrator reaction mass
                                      19 = Vibrator baseplate
                                      20 = Vibrator estimated ground force
                                      21 = Vibrator reference
                                      22  N = optional use, (maximum N = 32,767)*/
  short TraceNumVerSum;       /* 09  031-032
                                   Number of vertically summed traces yielding this trace. (1 is one trace, 2 is two
                                   summed traces, etc.)*/
  short TraceNumHorSum;       /* 10  033-034
                                   Number of horizontally stacked traces yielding this trace. (1 is one trace, 2 is
                                   two stacked traces, etc.)*/
  short DataType;             /* 11  035-036
                                   Data use:
                                      1 = Production
                                      2 = Test*/
  int   DistanceStoR;         /* 12  037-040
                                   Distance from center of the source point to the center of the receiver group
                                   (negzzative if opposite to direction in which line is shot).*/
  float ElevReceiver;         /* 13  041-044
                                   Receiver group elevation (all elevations above sea
                                   level are positive and below sea level are negative).*/
  float ElevSource;           /* 14  045-048
                                   Surface elevation at source.*/
  float DepthSource;          /* 15  049-052
                                   Source depth below surface (a positive number).
                                   Header bytes 69-70*/
  float ElevReceiverDatum;    /* 16  053-056
                                   Datum elevation at receiver group.*/
  float ElevSourceDatum;      /* 17  057-060
                                   Datum elevation at source.*/
  float DepthSourceWater;     /* 18  061-064
                                   Water depth at source.*/
  float DepthReceiverWater;   /* 19  065-068
                                   Water depth at group.*/
  short ScalerE;              /* 20  069-070 *
                                   Scalar to be applied to all elevations and depths specified in Trace Header bytes
                                   41-68 to give the real value. Scalar = 1, +10, +100, +1000, or +10,000. If
                                   positive, scalar is used as a multiplier; if negative, scalar is used as a divisor.*/
  short ScalerC;              /* 21  071-072
                                   Scalar to be applied to all coordinates specified in Trace Header bytes 73-88 and
                                   to bytes Trace Header 181-188 to give the real value. Scalar = 1, +10, +100,
                                   +1000, or +10,000. If positive, scalar is used as a multiplier; if negative,
                                   scalar is used as divisor.*/
  float SourceCoordinateX;    /* 22  073-076
                                   Source coordinate - X.*/
  float SourceCoordinateY;    /* 23  077-080
                                   Source coordinate - Y.*/
  float ReceiverCoordinateX;  /* 24  081-084
                                   Group coordinate - X.*/
  float ReceiverCoordinateY;  /* 25 085-088
                                   Group coordinate - Y.*/
  short CoordinateUnit;       /* 26  089-090
                                   Coordinate units:
                                       1 = Length (meters or feet)
                                       2 = Seconds of arc
                                       3 = Decimal degrees
                                       4 = Degrees, minutes, seconds
                                   Note: To encode �DDMMSS bytes 89-90 equal = �DD*10^4 + MM*10^2 + SS
                                   with bytes 71-72 set to 1; To encode �DDMMSS.ss bytes 89-90 equal =
                                   �DD*10^6 + MM*10^4 + SS*10^2 with bytes 71-72 set to -100.*/
  short WeatherVelocity;      /* 27  091-092
                                   Weathering velocity.
                                    (ft/s or m/s as specified in Binary File Header bytes 3255-3256)*/
  short SubweatherVelocity;   /* 28  093-094
                                   Subweathering velocity.
                                     (ft/s or m/s as specified in Binary File Header bytes 3255-3256)*/
  short UpholeTimeSource;     /* 29  095-096
                                   Uphole time at source in milliseconds.*/
  short UpholeTimeReceiver;   /* 30  097-098
                                   Uphole time at group in milliseconds.*/
  short StaticCorrectSource;  /* 31  099-100
                                   Source static correction in milliseconds.*/
  short StaticCorrectReceiver;/* 32  101-102
                                   Group static correction in*/
  short TotalStatic;          /* 33  103-104
                                   Total static applied in milliseconds. (Zero if no static has been applied,)*/
  short LagTimeA;             /* 34  105-106
                                   Lag time A  Time in milliseconds between end of 240-byte
                                   trace identification header and time break. The value is
                                   positive if time break occurs after the end of header; negative
                                   if time break occurs before the end of header. Time break is
                                   defined as the initiation pulse that may be recorded on an
                                   auxiliary trace or as otherwise specified by the recording system.*/
  short LagTimeB;             /* 35  107-108
                                   Lag Time B  Time in milliseconds between time break and
                                   the initiation time of the energy source. May be positive or negative.*/
  short DelayTime;            /* 36  109-110
                                   Delay recording time  Time in milliseconds between
                                   initiation time of energy source and the time when recording
                                   of data samples begins. In SEG Y rev 0 this entry was
                                   intended for deep-water work if data recording does not start
                                   at zero time. The entry can be negative to accommodate
                                   negative start times (i.e. data recorded before time zero,
                                   presumably as a result of static application to the data trace).
                                   If a non-zero value (negative or positive) is recorded in this
                                   entry, a comment to that effect should appear in the Textual
                                   File Header.*/
  short MuteTimeStart;        /* 37  111-112
                                   Mute time  Start time in milliseconds.*/
  short MuteTimeEnd;          /* 38  113-114
                                   Mute time  End time in milliseconds.*/
  short SampleNum;            /* 39  115-116
                                   Number of samples in this trace.*/
  short SampleInterval;       /* 40  117-118
                                   Sample interval in microseconds (�) for this trace.
                                   The number of bytes in a trace record must be consistent with the number of
                                   samples written in the trace header. This is important for all recording media;
                                   but it is particularly crucial for the correct processing of SEG Y data in disk
                                   files (see Appendix C).
                                   If the fixed length trace flag in bytes 3503-3504 of the Binary File Header is set,
                                   the sample interval and number of samples in every trace in the SEG Y file must
                                   be the same as the values recorded in the Binary File Header. If the fixed length
                                   trace flag is not set, the sample interval and number of samples may vary from
                                   trace to trace.*/
  short GainType;             /* 41  119-120
                                   Gain type of field instruments:
                                      1 = fixed
                                      2 = binary
                                      3 = floating point
                                      4  N = optional use*/
  short GainConstant;         /* 42   121-122
                                   Instrument gain constant.*/
  short InitalGain;           /* 43  123-124
                                   Instrument early or initial gain (dB).*/
  short Correlated;           /* 44  125-126
                                   Correlated:
                                      1 = no
                                      2 = yes*/
  short SweepFreqStart;       /* 45  127-128
                                   Sweep frequency at start (Hz).*/
  short SweepFreqEnd;         /* 46  129-130
                                   Sweep frequency at end (Hz).*/
  short SweepLength;          /* 47  131-132
                                   Sweep length in milliseconds.*/
  short SweepType;            /* 48  133-134
                                   Sweep type:
                                      1 = linear
                                      2 = parabolic
                                      3 = exponential
                                      4 = other*/
  short SweepTaperLStart;     /* 49  135-136
                                   Sweep trace taper length at start in milliseconds.*/
  short SweepTaperLEnd;       /* 50  137-138
                                   Sweep trace taper length at end in milliseconds.*/
  short SweepTaperType;       /* 51  139-140
                                   Taper type:
                                      1 = linear
                                      2 = cos^2
                                      3 = other*/
  short AliasFreq;            /* 52  141-142
                                   Alias filter frequency (Hz), if used.*/
  short AliasSlope;           /* 53  143-144
                                   Alias filter slope (dB/octave).*/
  short NotchFreq;            /* 54  145-146
                                   Notch filter frequency (Hz), if used.*/
  short NotchSlope;           /* 55  147-148
                                   Notch filter slope (dB/octave).*/
  short LowCutFreq;           /* 56  149-150
                                   Low-cut frequency (Hz), if used./*/
  short HighCutFreq;          /* 57  151-152
                                   High-cut frequency (Hz), if used.*/
  short LowCutSlope;          /* 58  153-154
                                   Low-cut slope (dB/octave)*/
  short HighCutSlope;         /* 59  155-156
                                   High-cut slope (dB/octave)*/
  short Year;                 /* 60  157-158
                                   Year data recorded  The 1975 standard is unclear as to whether this should be
                                   recorded as a 2-digit or a 4-digit year and both have been used. For SEG Y
                                   revisions beyond rev 0, the year should be recorded as the complete 4-digit
                                   Gregorian calendar year (i.e. the year 2001 should be recorded as 200110
                                   (7D116)).*/
  short Day;                  /* 61  159-160
                                   Day of year.*/
  short Hour;                 /* 62  161-162
                                   Hour of day (24 hour clock).*/
  short Minute;               /* 63   163-164
                                   Minute of hour.*/
  short Second;               /* 64  165-166
                                   Second of minute.*/
  short TimeBasisCode;        /* 65  167-168
                                   Time basis code:
                                       1 = Local
                                       2 = GMT (Greenwich Mean Time)
                                       3 = Other, should be explained in a user defined stanza
                                           in the Extended Textual File Header
                                       4 = UTC (Coordinated Universal Time)*/
  short TraceWeightFactor;    /* 66  169-170
                                   Trace weighting factor  Defined as 2 -N volts for the least significant bit.
                                   (N = 0,1, , 32767)*/
  short GeophoneNoRoll;       /* 67  171-172
                                   Geophone group number of roll switch position one.*/
  short GeophoneNoFirstTrace; /* 68  173-174
                                   Geophone group number of trace number one within original field record.*/
  short GeophoneNoLastTrace;  /* 69  175-176
                                   Geophone group number of last trace within original field record.*/
  short GapSize;              /* 70   177-178
                                   Gap size (total number of groups dropped).*/
  short Overtravel;           /* 71  179-180
                                   Over travel associated with taper at beginning or end of line:
                                       1 = down (or behind)
                                       2 = up (or ahead)*/
  float CDPPosX;              /* 72  181-184
                                   X coordinate of CDP position of this trace (scalar in Trace Header bytes 71-72
                                   applies).*/
  float CDPPosY;              /* 73  185-188
                                   Y coordinate of CDP position of this trace (scalar in bytes Trace Header 71-72
                                   applies).*/
  long InLineNo;              /* 74  189-192
                                   For 3-D poststack data, this field should be used for the in-line number. If one
                                   in-line per SEG Y file is being recorded, this value should be the same for all
                                   traces in the file and the same value will be recorded in bytes 3205-3208 of the
                                   Binary File Header.*/
  long CrossLineNo;           /* 75  193-196
                                   For 3-D poststack data, this field should be used for the cross-line number. This
                                   will typically be the same value as the CDP number in Trace Header bytes 21-
                                   24, but this doesnt have to be the case.*/
  float ShotPointNum;         /* 76  197-200
                                   Shotpoint number  This is probably only applicable to 2-D poststack data.
                                   Note that it is assumed that the shotpoint number refers to the source location
                                   nearest to the CDP location for a particular trace. If this is not the case, there
                                   should be a comment in the Textual Header explaining what the shotpoint
                                   number actually refers to.*/
  short ScaleS;               /* 77  201-202
                                   Scalar to be applied to the shotpoint number in Trace Header bytes 197-200 to
                                   give the real value. If positive, scalar is used as a multiplier; if negative as a
                                   divisor; if zero the shotpoint number is not scaled (i.e. it is an integer.A typical
                                   value will be -10, allowing shotpoint numbers with one place of decimals).*/
  short ValueUnit;            /* 78  203-204
                                   Trace value measurement unit:
                                       -1 = Other (should be described in Data Sample Measurement Units stanza)
                                        0 = Unknown
                                        1 = Pascal (Pa)
                                        2 = Volts (v)
                                        3 = Millivolts (mV)
                                        4 = Amperes (A)
                                        5 = Meters (m)
                                        6 = Meters per second (m/s)
                                        7 = Meters per second squared (m/s^2)
                                        8 = Newton (N)
                                        9 = Watt (W)*/
  long TransConstantB;        /* 79  205-210
                                   Transduction Constant  The multiplicative constant used to convert the Data
                                   Trace samples to the Transduction Units (specified in Trace Header bytes 211-
                                   212). The constant is encoded as a four-byte, two's complement integer (bytes
                                   205-208) which is the mantissa and a two-byte, two's complement integer (bytes
                                   209-210) which is the power of ten exponent (i.e. Bytes 205-208 * 10**Bytes
                                   209-210).*/
  short TransConstantE;
  short TransUnit;            /* 80  211-212
                                   Transduction Units  The unit of measurement of the Data Trace samples after
                                   they have been multiplied by the Transduction Constant specified in Trace
                                   Header bytes 205-210.
                                       -1 = Other (should be described in Data Sample Measurement Units stanza)
                                        0 = Unknown
                                        1 = Pascal (Pa)
                                        2 = Volts (v)
                                        3 = Millivolts (mV)
                                        4 = Amperes (A)
                                        5 = Meters (m)
                                        6 = Meters per second (m/s)
                                        7 = Meters per second squared (m/s^2)
                                        8 = Newton (N)
                                        9 = Watt (W)*/
  short DeviceID;             /* 81  213-214
                                   Device/Trace Identifier  The unit number or id number of the device associated
                                   with the Data Trace (i.e. 4368 for vibrator serial number 4368 or 20316 for gun
                                   16 on string 3 on vessel 2). This field allows traces to be associated across
                                   trace ensembles independently of the trace number (Trace Header bytes 25-28).*/
  short ScaleT;               /* 82  215-216
                                   Scalar to be applied to times specified in Trace Header bytes 95-114 to give the
                                   true time value in milliseconds. Scalar = 1, +10, +100, +1000, or +10,000. If
                                   positive, scalar is used as a multiplier; if negative, scalar is used as divisor. A
                                   value of zero is assumed to be a scalar value of 1.*/
  int  Reserved[6];           /* 83  217-240   */

}SEGY_TRACE_HEAD;



typedef union{
  char ch[4];
  unsigned char uc[4];

  int it;
  unsigned int ui;
  short int st[2];
  unsigned short int us[2];
  long int  lt;
  unsigned long ul;

  float ft;
}VALUE_TYPE;


/*-------------------------------------------------------------------------------------------------------------------------------------------*/
int get_file_length(FILE *fp);
void Swap4Char(char *ch);
float ibm_to_float(const float ibm);
float float_to_ibm(const float mfloat);
void ebcd_to_ascii( char *in, char *out,int num);
void ascii_to_ebcd( char *in, char *out,int num);

void get_segy_text_head (FILE *fp, char m_EBCD[40][80]);
void get_segy_bfile_head(FILE *fp, SEGY_BFILE_HEAD *m_Hd);
int  get_segy_trace_head(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, SEGY_TRACE_HEAD *m_Tr);
int  get_segy_trace_data(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, float **data);
int  get_segy_cdp_data  (FILE *fp, SEGY_BFILE_HEAD bfh, int firstcdp, int endcdp, float ***trdata);
int  get_segy_cdp_data1 (FILE *fp, SEGY_BFILE_HEAD bfh, int NO, float ***data);
int  get_segy_line_data (FILE *fp, SEGY_BFILE_HEAD bfh, float ***trdata);
int  get_segy_line_data2(FILE *fp, SEGY_BFILE_HEAD bfh, float ***trdata,int BeginTrace,int EndTrace);

void put_segy_text_head (FILE *fp, char str[40][80] );
void put_segy_bfile_head(FILE *fp, SEGY_BFILE_HEAD m_Hd);
void put_segy_trace_head(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, SEGY_TRACE_HEAD m_Tr);
void put_segy_trace_data(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, float *data);
void put_array_in_segy  (FILE *fp, int format,int TrNum, int num, float dx, float dt, float **data);

void print_segy_text_head (FILE *fp, char m_EBCD[40][80]);
void print_segy_bfile_head(FILE *fp, SEGY_BFILE_HEAD m_Hd);
void print_segy_trace_head(FILE *fp,int NO, SEGY_TRACE_HEAD m_Tr);
void print_trace_data     (FILE *fp, int TrNum, int num, float **data);

int  get_segy_trace_num(FILE *fp, SEGY_BFILE_HEAD fhd);
void get_segy_cdp_no(FILE *fp, SEGY_BFILE_HEAD bfh, int **cdpno);

void get_segy_trace_data1(FILE *fp, unsigned short Rev, int format,int len_flag,int num, int NO, float *data);
void put_segy_trace_data1(FILE *fp, unsigned short Rev, int format,int len_flag,int num, int NO, float *data);

void get_segy_trace(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, SEGY_TRACE_HEAD *m_Tr, float *tr);
void put_segy_trace(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, SEGY_TRACE_HEAD m_Tr,  float *tr);

void get_segy_trace_all(FILE *fp, SEGY_BFILE_HEAD bfh, SEGY_TRACE_HEAD *tr, int FirTr, int EndTr, float **data);
void put_segy_trace_all(FILE *fp, SEGY_BFILE_HEAD bfh, SEGY_TRACE_HEAD *tr, int FirTr, int EndTr, float **data);

int get_segy_sorce_XY(FILE *fp, SEGY_BFILE_HEAD bfh, float ***XY);

int  check_segy_trace_len(const char *filename);

/*------------------------------------------------------------------------------------------------------------------------------------------------*/

void put_trace(FILE *fp, float *tr, int num, int NO);
void get_trace(FILE *fp, float *tr, int num, int NO);
void put_line(FILE *fp, float **data, int TrNum, int num, int line);
void get_line(FILE *fp, float **data, int TrNum, int num, int line);

/*------------------------------------------------------------------------------------------------------------------------------------------------*/

int get_su_trace_head(FILE *fp, segy *tr,int NO,int nl);
int get_segy_cdpdata(int fp, SEGY_BFILE_HEAD bfh, int firstcdp, int endcdp, float ***trdata);
int get_su_line_data2(FILE *fp, segy *tr,float ***trdata, int BeginTrace, int EndTrace);
//---------------------------------------------------------------------------
int get_segy_cdp_data2(FILE *fp, SEGY_BFILE_HEAD bfh, int firstcdp, int endcdp, float ***trdata);
int get_segy_cdp_trace(FILE *fp, SEGY_BFILE_HEAD bfh,int NO);
int get_segy_cdp_total_trace(FILE *fp, SEGY_BFILE_HEAD bfh, int firstcdp, int endcdp);
int cdp_number_trace(int *cdpno,int tracenum,int **cdp);
void cdp_min_max(int *cdpno,int tracenum,int min,int max);
#endif /* segy.h */
