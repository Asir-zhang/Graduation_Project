#include <clx.h>
#pragma hdrstop

#include <sys/types.h>
#include <sys/stat.h>

#include "segy.h"
#include "base.h"
#include "segy_su.h"
#include <io.h>
int get_file_length(FILE *fp)
{
  struct stat statbuf;

  int n=fstat(fileno(fp), &statbuf);

  return statbuf.st_size;

}

void Swap4Char(char *ch)
{
  char cc;
  cc = ch[0];    ch[0] = ch[3];    ch[3] = cc;
  cc = ch[1];    ch[1] = ch[2];    ch[2] = cc;
  return;
}


float ibm_to_float(const float ibm)
{

  int s,fmant;
  long mlong;
  VALUE_TYPE mm;

  mm.ft = ibm;
/*
  mlong = ((long)m.ch[1] << 16) + ((long)m.ch[2] << 8) + m.ch[3];
  mfloat = float(mlong) / float(0x1000000);
  mfloat =(float)( mfloat * pow(16.0,double((m.ch[0] & 0x7F) - 64)) );
  if (m.ch[0] & 0x80)  return -mfloat;
  else                  return mfloat;
*/

  Swap4Char(mm.ch);
   mlong = mm.lt;
  fmant = 0x00ffffff & mlong;
  if (!fmant){
    mlong=0;
  }else{
    s = (int) ((0x7f000000 & mlong) >> 22) - 130;
    while (!(fmant & 0x00800000)) { --s; fmant <<= 1; }
    if (s > 254) mlong = (0x80000000 & mlong) | 0x7f7fffff;
    else if (s <= 0) mlong = 0;
    else mlong = (0x80000000 & mlong) |(s << 23)|(0x007fffff & fmant);
  }
  mm.lt = mlong;

    return mm.ft;
}


float float_to_ibm(const float mfloat)
{
  float ibm=100.23f;
  long mlong;
  int s,fmant;
  union{
    char  ch[4];
    long  li;
    float ff;
  }m;

  m.ff = mfloat;
  mlong = m.li;
  if (mlong) {
    fmant = (0x007fffff & mlong) | 0x00800000;
    s = (int) ((0x7f800000 & mlong) >> 23) - 126;
    while (s & 0x3) { ++s; fmant >>= 1; }
    mlong = (0x80000000 & mlong) | (((s>>2) + 64) << 24) | fmant;
  }
  m.li = mlong;
  Swap4Char(m.ch);

  ibm = m.ff;
  return ibm;
}

void ebcd_to_ascii( char *in, char *out,int num)
{
  /* Initialized data */
  static int t1[96] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
    48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,
    70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,
    92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,
    110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
    126,127 };
  static int t2[96] = { 64,79,127,123,91,108,80,125,77,93,92,78,107,96,
    75,97,240,241,242,243,244,245,246,247,248,249,122,94,76,126,110,
    111,124,193,194,195,196,197,198,199,200,201,209,210,211,212,213,
    214,215,216,217,226,227,228,229,230,231,232,233,74,224,90,95,108,
    64,129,130,131,132,133,134,135,136,137,145,146,147,148,149,150,
    151,152,153,162,163,164,165,166,167,168,169,192,106,208,161,64 };

  /* Local variables */
  unsigned char tchar;
  int i, k, kk;

  for( i=0; i<num; ++i ){
    tchar = ' ';
    kk = (unsigned char)in[i];
    for( k=0; k<96; ++k ){
      if( kk == t2[k] ){  tchar = t1[k];  break;  }
    }
    out[i] = (char)tchar;
  }

  return;
}

void ascii_to_ebcd( char *in, char *out,int num)
{
    /* Initialized data */
    static int t1[96] = { 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
      48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,
      70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,
      92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,
      110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
      126,127 };
    static int t2[96] = { 64,79,127,123,91,108,80,125,77,93,92,78,107,96,
      75,97,240,241,242,243,244,245,246,247,248,249,122,94,76,126,110,
      111,124,193,194,195,196,197,198,199,200,201,209,210,211,212,213,
      214,215,216,217,226,227,228,229,230,231,232,233,74,224,90,95,108,
      64,129,130,131,132,133,134,135,136,137,145,146,147,148,149,150,
      151,152,153,162,163,164,165,166,167,168,169,192,106,208,161,64 };

    /* Local variables */
    unsigned char tchar;
    int i, k, kk;

    for ( i=0; i<num;++i ){
    tchar = ' ';
    kk =  (unsigned char)in[i];
    for ( k=0;k<96;++k ){
      if ( kk==t1[k]){  tchar = t2[k];  break;  }
    }
    out[i] = tchar;
    }

    return;
}


void get_segy_text_head(FILE *fp, char m_EBCD[40][80])
{
  int i;
  char temchar[80];

  fseek(fp,0,0);
  for( i=0;i<40;i++ ){
    fread(temchar,1,80,fp);
    ebcd_to_ascii(temchar,m_EBCD[i],80);
  }
  return;
}

void get_segy_bfile_head(FILE *fp, SEGY_BFILE_HEAD *m_Hd)
/*-----------------------------------------------------------------------------------------------------------------------------------------
         Input:
             fp         The pointer to the file readed
        OutPut:
             m_Hd    The SEGY binary file head, lies 3200 to 3600 character
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int i,traceh[100];
  long count;
  VALUE_TYPE mm;

  count = 3200L;
  fseek(fp,count,0);
  fread(traceh,4,100,fp);
  mm.it = traceh[0];    Swap4Char(mm.ch);    m_Hd->NoJob = mm.lt;
  mm.it = traceh[1];    Swap4Char(mm.ch);    m_Hd->NoLine = mm.lt;
  mm.it = traceh[2];    Swap4Char(mm.ch);    m_Hd->NoReel = mm.lt;
  mm.it = traceh[3];    Swap4Char(mm.ch);    m_Hd->NumDataTrace = mm.st[1];    m_Hd->NumAuxiTrace = mm.st[0];
  mm.it = traceh[4];    Swap4Char(mm.ch);    m_Hd->SampleRatioReel = mm.st[1];  m_Hd->SampleRatioField = mm.st[0];
  mm.it = traceh[5];    Swap4Char(mm.ch);    m_Hd->SampleNumReel = mm.st[1];    m_Hd->SampleNumField = mm.st[0];
  mm.it = traceh[6];    Swap4Char(mm.ch);    m_Hd->DataFormat = mm.st[1];      m_Hd->CDPFold = mm.st[0];
  mm.it = traceh[7];    Swap4Char(mm.ch);    m_Hd->TraceSortCode = mm.st[1];    m_Hd->VerticalSumCode = mm.st[0];
  mm.it = traceh[8];    Swap4Char(mm.ch);    m_Hd->SweepFreqStart = mm.st[1];    m_Hd->SweepFreqEnd = mm.st[0];
  mm.it = traceh[9];    Swap4Char(mm.ch);    m_Hd->SweepLength = mm.st[1];    m_Hd->SweepTypeCode = mm.st[0];
  mm.it = traceh[10];    Swap4Char(mm.ch);    m_Hd->SweepTraceNum = mm.st[1];    m_Hd->SweepTaperLenStart = mm.st[0];
  mm.it = traceh[11];    Swap4Char(mm.ch);    m_Hd->SweepTaperLenEnd = mm.st[1];   m_Hd->SweepTaperType = mm.st[0];
  mm.it = traceh[12];    Swap4Char(mm.ch);    m_Hd->CorrelDataTrace = mm.st[1];  m_Hd->BinaryGainRecover = mm.st[0];
  mm.it = traceh[13];    Swap4Char(mm.ch);    m_Hd->AmpRecoverMethod = mm.st[1];   m_Hd->MeasurementSys = mm.st[0];
  mm.it = traceh[14];    Swap4Char(mm.ch);    m_Hd->ImpulseSignal = mm.st[1];    m_Hd->VibratoryPolarityCode = mm.st[0];
  for( i=0;i<60;i++ )  m_Hd->Reserved1[i] = traceh[15+i];
  mm.it = traceh[75];    Swap4Char(mm.ch);    m_Hd->Revision = mm.st[1];      m_Hd->TraceLengthFlag = mm.st[0];
  mm.it = traceh[76];    Swap4Char(mm.ch);    m_Hd->ExtTextNum = mm.st[1];

  /* Set TraceLengthFlag = 1 for the old SEGY file , ie.  Revision=0 */
  if(  m_Hd->Revision==0 )  m_Hd->TraceLengthFlag =1;

  return;
}


int get_segy_trace_head(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, SEGY_TRACE_HEAD *m_Tr)
/*-----------------------------------------------------------------------------------------------------------------------------------------
    Input:
        fp          The pointer to the file readed
        bfh         The SEGY file binary file head
        NO          The trace sequence number in file
     Output:
        m_Tr        The SEGY trace head struct
     Return:
        num         The number of sample each trace if len_flag = 1.
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int traceh[60];
  int count;
  int temp,num;
  VALUE_TYPE mm;

  if( bfh.DataFormat==0 )  return 0;

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  if( bfh.TraceLengthFlag==1 )
  {
    num = bfh.SampleNumReel;
    count = 3600L+(temp*num+240L)*(NO-1);
  }
  else
  {
    return 0;
  }

  fseek(fp,count,0);
  fread(traceh,4,60,fp);
  mm.it = traceh[0];  Swap4Char(mm.ch);   m_Tr->TraceNoLine = mm.it;
  mm.it = traceh[1];  Swap4Char(mm.ch);   m_Tr->TraceNoReel = mm.it;
  mm.it = traceh[2];  Swap4Char(mm.ch);   m_Tr->RecordNoField = mm.it;
  mm.it = traceh[3];  Swap4Char(mm.ch);   m_Tr->TraceNoField = mm.it;
  mm.it = traceh[4];  Swap4Char(mm.ch);   m_Tr->SourceNo = mm.it;
  mm.it = traceh[5];  Swap4Char(mm.ch);   m_Tr->CDPNo = mm.it;
  mm.it = traceh[6];  Swap4Char(mm.ch);   m_Tr->TraceNoCDP = mm.it;
  mm.it = traceh[7];  Swap4Char(mm.ch);   m_Tr->TraceCode = mm.st[1];        m_Tr->TraceNumVerSum = mm.st[0];
  mm.it = traceh[8];  Swap4Char(mm.ch);   m_Tr->TraceNumHorSum = mm.st[1];   m_Tr->DataType = mm.st[0];
  mm.it = traceh[9];  Swap4Char(mm.ch);   m_Tr->DistanceStoR = mm.it;

  mm.it = traceh[17]; Swap4Char(mm.ch);   m_Tr->ScalerE = mm.st[1];           m_Tr->ScalerC = mm.st[0];
  if( m_Tr->ScalerE>0 ){
    mm.it = traceh[10]; Swap4Char(mm.ch);   m_Tr->ElevReceiver = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[11]; Swap4Char(mm.ch);   m_Tr->ElevSource = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[12]; Swap4Char(mm.ch);   m_Tr->DepthSource = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[13]; Swap4Char(mm.ch);   m_Tr->ElevReceiverDatum = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[14]; Swap4Char(mm.ch);   m_Tr->ElevSourceDatum = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[15]; Swap4Char(mm.ch);   m_Tr->DepthSourceWater = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[16]; Swap4Char(mm.ch);   m_Tr->DepthReceiverWater = (float)mm.lt*m_Tr->ScalerE;
  }else if( m_Tr->ScalerE<0 ){
    mm.it = traceh[10]; Swap4Char(mm.ch);   m_Tr->ElevReceiver = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[11]; Swap4Char(mm.ch);   m_Tr->ElevSource = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[12]; Swap4Char(mm.ch);   m_Tr->DepthSource = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[13]; Swap4Char(mm.ch);   m_Tr->ElevReceiverDatum = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[14]; Swap4Char(mm.ch);   m_Tr->ElevSourceDatum = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[15]; Swap4Char(mm.ch);   m_Tr->DepthSourceWater = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[16]; Swap4Char(mm.ch);   m_Tr->DepthReceiverWater = (float)mm.lt/m_Tr->ScalerE;
  }else{
    mm.it = traceh[10]; Swap4Char(mm.ch);   m_Tr->ElevReceiver = (float)mm.it;
    mm.it = traceh[11]; Swap4Char(mm.ch);   m_Tr->ElevSource = (float)mm.it;
    mm.it = traceh[12]; Swap4Char(mm.ch);   m_Tr->DepthSource = (float)mm.it;
    mm.it = traceh[13]; Swap4Char(mm.ch);   m_Tr->ElevReceiverDatum = (float)mm.it;
    mm.it = traceh[14]; Swap4Char(mm.ch);   m_Tr->ElevSourceDatum = (float)mm.it;
    mm.it = traceh[15]; Swap4Char(mm.ch);   m_Tr->DepthSourceWater = (float)mm.it;
    mm.it = traceh[16]; Swap4Char(mm.ch);   m_Tr->DepthReceiverWater = (float)mm.it;
  }

  if( m_Tr->ScalerC>0 ){
    mm.it = traceh[18]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateX = (float)mm.lt*m_Tr->ScalerC;
    mm.it = traceh[19]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateY = (float)mm.lt*m_Tr->ScalerC;
    mm.it = traceh[20]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateX = (float)mm.lt*m_Tr->ScalerC;
    mm.it = traceh[21]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateY = (float)mm.lt*m_Tr->ScalerC;
  }else if( m_Tr->ScalerC<0 ){
    mm.it = traceh[18]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateX = (float)mm.lt/m_Tr->ScalerC;
    mm.it = traceh[19]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateY = (float)mm.lt/m_Tr->ScalerC;
    mm.it = traceh[20]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateX = (float)mm.lt/m_Tr->ScalerC;
    mm.it = traceh[21]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateY = (float)mm.lt/m_Tr->ScalerC;
  }else{
    mm.it = traceh[18]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateX = (float)mm.it;
    mm.it = traceh[19]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateY = (float)mm.it;
    mm.it = traceh[20]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateX = (float)mm.it;
    mm.it = traceh[21]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateY = (float)mm.it;
  }

  mm.it = traceh[22]; Swap4Char(mm.ch);   m_Tr->CoordinateUnit = mm.st[1];        m_Tr->WeatherVelocity = mm.st[0];
  mm.it = traceh[23]; Swap4Char(mm.ch);   m_Tr->SubweatherVelocity = mm.st[1];    m_Tr->UpholeTimeSource = mm.st[0];
  mm.it = traceh[24]; Swap4Char(mm.ch);   m_Tr->UpholeTimeReceiver = mm.st[1];    m_Tr->StaticCorrectSource = mm.st[0];
  mm.it = traceh[25]; Swap4Char(mm.ch);   m_Tr->StaticCorrectReceiver = mm.st[1]; m_Tr->TotalStatic = mm.st[0];
  mm.it = traceh[26]; Swap4Char(mm.ch);   m_Tr->LagTimeA = mm.st[1];              m_Tr->LagTimeB = mm.st[0];
  mm.it = traceh[27]; Swap4Char(mm.ch);   m_Tr->DelayTime = mm.st[1];             m_Tr->MuteTimeStart = mm.st[0];
  mm.it = traceh[28]; Swap4Char(mm.ch);   m_Tr->MuteTimeEnd = mm.st[1];           m_Tr->SampleNum = mm.st[0];
  mm.it = traceh[29]; Swap4Char(mm.ch);   m_Tr->SampleInterval = mm.st[1];        m_Tr->GainType = mm.st[0];
  mm.it = traceh[30]; Swap4Char(mm.ch);   m_Tr->GainConstant = mm.st[1];          m_Tr->InitalGain = mm.st[0];
  mm.it = traceh[31]; Swap4Char(mm.ch);   m_Tr->Correlated = mm.st[1];            m_Tr->SweepFreqStart = mm.st[0];
  mm.it = traceh[32]; Swap4Char(mm.ch);   m_Tr->SweepFreqEnd = mm.st[1];          m_Tr->SweepLength = mm.st[0];
  mm.it = traceh[33]; Swap4Char(mm.ch);   m_Tr->SweepType = mm.st[1];             m_Tr->SweepTaperLStart = mm.st[0];
  mm.it = traceh[34]; Swap4Char(mm.ch);   m_Tr->SweepTaperLEnd = mm.st[1];        m_Tr->SweepTaperType = mm.st[0];
  mm.it = traceh[35]; Swap4Char(mm.ch);   m_Tr->AliasFreq = mm.st[1];             m_Tr->AliasSlope = mm.st[0];
  mm.it = traceh[36]; Swap4Char(mm.ch);   m_Tr->NotchFreq = mm.st[1];             m_Tr->NotchSlope = mm.st[0];
  mm.it = traceh[37]; Swap4Char(mm.ch);   m_Tr->LowCutFreq = mm.st[1];            m_Tr->HighCutFreq = mm.st[0];
  mm.it = traceh[38]; Swap4Char(mm.ch);   m_Tr->LowCutSlope = mm.st[1];           m_Tr->HighCutSlope = mm.st[0];
  mm.it = traceh[39]; Swap4Char(mm.ch);   m_Tr->Year = mm.st[1];                  m_Tr->Day = mm.st[0];
  mm.it = traceh[40]; Swap4Char(mm.ch);   m_Tr->Hour = mm.st[1];                  m_Tr->Minute = mm.st[0];
  mm.it = traceh[41]; Swap4Char(mm.ch);   m_Tr->Second = mm.st[1];                m_Tr->TimeBasisCode = mm.st[0];
  mm.it = traceh[42]; Swap4Char(mm.ch);   m_Tr->TraceWeightFactor = mm.st[1];     m_Tr->GeophoneNoRoll = mm.st[0];
  mm.it = traceh[43]; Swap4Char(mm.ch);   m_Tr->GeophoneNoFirstTrace = mm.st[1];  m_Tr->GeophoneNoLastTrace = mm.st[0];
  mm.it = traceh[44]; Swap4Char(mm.ch);   m_Tr->GapSize = mm.st[1];               m_Tr->Overtravel = mm.st[0];
  mm.it = traceh[45]; Swap4Char(mm.ch);   m_Tr->CDPPosX = (float)mm.lt;
  mm.it = traceh[46]; Swap4Char(mm.ch);   m_Tr->CDPPosY = (float)mm.lt;
  mm.it = traceh[47]; Swap4Char(mm.ch);   m_Tr->InLineNo = mm.lt;
  mm.it = traceh[48]; Swap4Char(mm.ch);   m_Tr->CrossLineNo = mm.lt;
  mm.it = traceh[49]; Swap4Char(mm.ch);   m_Tr->ShotPointNum = mm.ft;
  mm.it = traceh[50]; Swap4Char(mm.ch);   m_Tr->ScaleS = mm.st[1];                m_Tr->ValueUnit = mm.st[0];
  mm.it = traceh[51]; Swap4Char(mm.ch);   m_Tr->TransConstantB = mm.lt;
  mm.it = traceh[52]; Swap4Char(mm.ch);   m_Tr->TransConstantE = mm.st[1];        m_Tr->TransUnit = mm.st[0];
  mm.it = traceh[53]; Swap4Char(mm.ch);   m_Tr->DeviceID = mm.st[1];              m_Tr->ScaleT = mm.st[0];
  m_Tr->Reserved[0] = traceh[54];
  m_Tr->Reserved[1] = traceh[55];
  m_Tr->Reserved[2] = traceh[56];
  m_Tr->Reserved[3] = traceh[57];
  m_Tr->Reserved[4] = traceh[58];
  m_Tr->Reserved[5] = traceh[59];

  if( bfh.Revision==0x0100 ){
    if( m_Tr->ScaleT>0 ){
      m_Tr->CDPPosX *= m_Tr->ScalerC;
      m_Tr->CDPPosY *= m_Tr->ScalerC;
      m_Tr->ShotPointNum *= m_Tr->ScalerC;
      m_Tr->UpholeTimeSource *= m_Tr->ScaleT;
      m_Tr->UpholeTimeSource *= m_Tr->ScaleT;
      m_Tr->UpholeTimeReceiver *= m_Tr->ScaleT;
      m_Tr->StaticCorrectSource *= m_Tr->ScaleT;
      m_Tr->StaticCorrectReceiver *= m_Tr->ScaleT;
      m_Tr->TotalStatic *= m_Tr->ScaleT;
      m_Tr->LagTimeA *= m_Tr->ScaleT;
      m_Tr->LagTimeB *= m_Tr->ScaleT;
      m_Tr->DelayTime *= m_Tr->ScaleT;
      m_Tr->MuteTimeStart *= m_Tr->ScaleT;
      m_Tr->MuteTimeEnd *= m_Tr->ScaleT;
    }else if( m_Tr->ScaleT<0 ){
      m_Tr->CDPPosX /= -m_Tr->ScalerC;
      m_Tr->CDPPosY /= -m_Tr->ScalerC;
      m_Tr->ShotPointNum /= -m_Tr->ScalerC;
      m_Tr->UpholeTimeSource /= -m_Tr->ScaleT;
      m_Tr->UpholeTimeSource /= -m_Tr->ScaleT;
      m_Tr->UpholeTimeReceiver /= -m_Tr->ScaleT;
      m_Tr->StaticCorrectSource /= -m_Tr->ScaleT;
      m_Tr->StaticCorrectReceiver /= -m_Tr->ScaleT;
      m_Tr->TotalStatic /= -m_Tr->ScaleT;
      m_Tr->LagTimeA /= -m_Tr->ScaleT;
      m_Tr->LagTimeB /= -m_Tr->ScaleT;
      m_Tr->DelayTime /= -m_Tr->ScaleT;
      m_Tr->MuteTimeStart /= -m_Tr->ScaleT;
      m_Tr->MuteTimeEnd /= -m_Tr->ScaleT;
    }else{

    }
  }

  return num;
}


int get_segy_trace_data(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, float **trdata)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp          The pointer to the file readed
        bfh         The SEGY file binary file head
        NO          The trace sequence number in file
     Output:
        trdata      The SEGY trace data
     Return:
        num         The number of sample each trace if len_flag = 1.
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int i,num;
  long count,temp;
  float *data=NULL;
  VALUE_TYPE mm;

  if( bfh.DataFormat==0 )  return 0;

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  if( bfh.TraceLengthFlag==1 ){
    num = bfh.SampleNumReel;
    count = 3840L+(temp*num+240L)*(NO-1);
  }else{
    return 0;
  }

  if( *trdata!=NULL ) Free1Float(*trdata);
  data = Alloc1Float(num);

  fseek(fp,count,0);
  fread(data,temp,num,fp);
  for( i=0;i<num;i++ ){
    mm.ft = data[i];
    switch( bfh.DataFormat ){
      case 1:  /*IBM 4 Bytes Floating*/
          data[i] = ibm_to_float(mm.ft);
          break;
      case 2:  /* IBM 4 Bytes Fixed Point*/
          data[i] = 0.0;
          break;
      case 3:  /* IBM 2 Bytes Fixed Point*/
          data[i] = 0.0;
          break;
      case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
          data[i] = 0.0;
          break;
      case 5:  /* IEEE32 4 Bytes Floating*/
          Swap4Char(mm.ch);
          data[i] = (float)(mm.ft);
          break;
      default:
          data[i] = 0.0;
    }
  }

  *trdata = data;
  return num;
}

int get_segy_cdp_data(FILE *fp, SEGY_BFILE_HEAD bfh, int firstcdp, int endcdp, float ***trdata)
/*-----------------------------------------------------------------------------------------------------------------------------------------
      Get all data in the same CDP from opened file.
        If NO<1, get the data in the first CDP.  If NO>maxcdpnum in file,get the data in the last CDP.
-------------------------------------------------------------------------------------------------------------------------------------------
        Input:
             fp      The pointer to the file read
             bfh     The SEGY binary file head, lies 3200 to 3600 character
         firstcdp    The first cdp number in file
          endcdp     The last cdp number in file
        Output:
             trdata  The data in same cdp trace
        Return:
             trnum   The number of trace in the cdp
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int   i,j,temp,trnum, trnum0, cdpnum;
  long  move,move0,tmcdp,ml, filelen;
  float **data;
  VALUE_TYPE mm;

  if( firstcdp>endcdp ) endcdp = firstcdp;
  if( endcdp<=0 ) endcdp = 100000;  /* read all cdp data */

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  filelen = get_file_length(fp);

  tmcdp = 0;

  ml = temp*bfh.SampleNumReel+240;
  move = 3600l+bfh.ExtTextNum*3200l;
  move0 = move+240;

  if( *trdata!=NULL ){ Free2Float(*trdata); *trdata=NULL; }

  trnum = 0;
  trnum0 = 0;
  cdpnum = 0;
  if( bfh.TraceLengthFlag==1 ){

   /* Count the number of trace in same CDP */
   while( !feof(fp) ){
      fseek(fp, move+20, SEEK_SET);
      fread(&mm.lt,4,1,fp);         Swap4Char(mm.ch);  tmcdp = mm.it;
      if( tmcdp<firstcdp ){
        trnum0++;
        move += ml;
        if( move>=filelen ) break;
        continue;
      }else if( tmcdp>endcdp ){
        break;
      }else{
        trnum++;
        move += ml;
        if( move>=filelen ) break;
      }
    }

    if( trnum==0 ){
      fprintf(stderr," Can't find the CDP number between %d and %d in the file\n",firstcdp,endcdp);
      fprintf(stderr," The first CDP number is %ld in the file\n",tmcdp);
      return trnum;
    }

/*    fprintf(stderr,"trnum0:%d    trnum:%d\n",trnum0,trnum);*/

    data = Alloc2Float(trnum, bfh.SampleNumReel);
    move0 += trnum0*ml;
    /**** Read all data in same CDP ****/
    for( i=0;i<trnum;i++ ){
      fseek(fp,move0,SEEK_SET);
      fread(data[i],4,bfh.SampleNumReel,fp);
      for( j=0;j<bfh.SampleNumReel;j++ ){
        mm.ft = data[i][j];
        switch( bfh.DataFormat ){
          case 1:  /*IBM 4 Bytes Floating*/
                 data[i][j] = ibm_to_float(mm.ft);
                 break;
          case 2:  /* IBM 4 Bytes Fixed Point*/
                 data[i][j] = 0.0;
                 break;
          case 3:  /* IBM 2 Bytes Fixed Point*/
                 data[i][j] = 0.0;
                 break;
          case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
                 data[i][j] = 0.0;
                 break;
          case 5:  /* IEEE32 4 Bytes Floating*/
                 Swap4Char(mm.ch);
                 data[i][j] = (float)(mm.ft);
                 break;
          default:
                 data[i][j] = 0.0;
        }
      }
      move0 += ml;
    }
  }else{ /* trace length is vary */

    data = Alloc2Float(2, bfh.SampleNumReel);

  }

  *trdata = data;

  return trnum;
}


int get_segy_cdp_data1(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, float ***trdata)
/*-----------------------------------------------------------------------------------------------------------------------------------------
      Get all data in the same CDP from opened file.
        If NO<1, get the data in the first CDP.  If NO>maxcdpnum in file,get the data in the last CDP.
-------------------------------------------------------------------------------------------------------------------------------------------
        Input:
             fp      The pointer to the file read
             bfh     The SEGY binary file head, lies 3200 to 3600 character
             NO      The cdp sequence number in file
        Output:
             trdata  The data in same cdp trace
        Return:
             trnum   The number of trace in the cdp
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int   i,j,temp,trnum, trnum0, cdpnum, cdp;
  long  move,move0,tmcdp,ml, filelen;
  float **data;
  VALUE_TYPE mm;

  if( NO<1 ) NO = 1;

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  filelen = get_file_length(fp);

  ml = temp*bfh.SampleNumReel;
  move = 3600l+bfh.ExtTextNum*3200l;
  move0 = move+240;

  if( *trdata!=NULL ) Free2Float(*trdata);

  trnum = 0;
  trnum0 = 0;
  cdpnum = 0;
  cdp = -10;
  if( bfh.TraceLengthFlag==1 ){

    /* Count the number of trace in same CDP */
   while( !feof(fp) ){
      fseek(fp, move+20, SEEK_SET);
      fread(&mm.lt,4,1,fp);         Swap4Char(mm.ch);  tmcdp = mm.it;
      if( tmcdp!=cdp ){
        cdp = tmcdp;
        cdpnum++;
        if( cdpnum==NO+1 ){ break; }
        else{ trnum0 += trnum; trnum = 0;   }
      }
      move += ml+240;
      trnum++;
      if( move>=filelen ) break;
    }

    data = Alloc2Float(trnum, bfh.SampleNumReel);
    move0 += trnum0*(ml+240);
    /**** Read all data in same CDP ****/
    for( i=0;i<trnum;i++ ){
      fseek(fp,move0,SEEK_SET);
      fread(data[i],4,bfh.SampleNumReel,fp);
      for( j=0;j<bfh.SampleNumReel;j++ ){
        mm.ft = data[i][j];
        switch( bfh.DataFormat ){
          case 1:  /*IBM 4 Bytes Floating*/
                 data[i][j] = ibm_to_float(mm.ft);
                 break;
          case 2:  /* IBM 4 Bytes Fixed Point*/
                 data[i][j] = 0.0;
                 break;
          case 3:  /* IBM 2 Bytes Fixed Point*/
                 data[i][j] = 0.0;
                 break;
          case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
                 data[i][j] = 0.0;
                 break;
          case 5:  /* IEEE32 4 Bytes Floating*/
                 Swap4Char(mm.ch);
                 data[i][j] = (float)(mm.ft);
                 break;
          default:
                 data[i][j] = 0.0;
        }
      }
      move0 += ml+240;
    }
  }else{ /* trace length is vary */

    data = Alloc2Float(2, bfh.SampleNumReel);

  }

  *trdata = data;

  return trnum;
}



int  get_segy_line_data(FILE *fp, SEGY_BFILE_HEAD bfh,  float ***trdata)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp          The pointer to the file readed
        bfh         The SEGY file binary file head
     Output:
        trdata      The SEGY data in same line. data[TrNum][num]
     Return:
        TrNum       The number of trace in the line.
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int  i,j,temp;
  int  TrNum,num;
  long count,ml;
  float **data;
  VALUE_TYPE mm;

  if( bfh.DataFormat == 3 ) temp = 2;
  else if( bfh.DataFormat ==8 ) temp = 1;
  else temp = 4;

  ml = temp*bfh.SampleNumReel;

  if( *trdata!=NULL ) Free2Float(*trdata);

  TrNum = get_segy_trace_num(fp, bfh);

  count = 3840L;
  if( bfh.TraceLengthFlag==0 )
  {

   data = Alloc2Float(TrNum,bfh.SampleNumReel);

    for( i=0;i<TrNum;i++)
    {
      fseek(fp,count-128,0);
      fread(&mm.it,4,1,fp);  Swap4Char(mm.ch);  num = (int)mm.st[0];
      fseek(fp,count,0);
      fread(data[i],temp,num,fp);
      for( j=0;j<num;j++ )
      {

        mm.ft = data[i][j];
        switch( bfh.DataFormat )
        {
          case 1:  /*IBM 4 Bytes Floating*/
              data[i][j] = ibm_to_float(mm.ft);
              break;
          case 2:   /*IBM 4 Bytes Fixed Point*/
              data[i][j] = 0.0;
              break;
          case 3:  /* IBM 2 Bytes Fixed Point*/
              data[i][j] = 0.0;
              break;
          case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
              data[i][j] = 0.0;
              break;
          case 5:  /* IEEE32 4 Bytes Floating*/
              Swap4Char(mm.ch);
              data[i][j] = (float)(mm.ft);
              break;
          default:
              data[i][j] = 0.0;
        }
      }
      count += num*temp+240L;
    }
  }
  else
  {

    data = Alloc2Float(TrNum,bfh.SampleNumReel);

    for( i=0;i<TrNum;i++)
    {

      fseek(fp,count,0);
      fread(data[i],temp,bfh.SampleNumReel,fp);
      for( j=0;j<bfh.SampleNumReel;j++ )
      {
        mm.ft = data[i][j];
        switch( bfh.DataFormat )
        {
          case 1:  /*IBM 4 Bytes Floating*/
              data[i][j] = ibm_to_float(mm.ft);
              break;
          case 2:  /* IBM 4 Bytes Fixed Point*/
              data[i][j] = 0.0;
              break;
          case 3: /*  IBM 2 Bytes Fixed Point*/
              data[i][j] = 0.0;
              break;
          case 4: /*  IBM 4 Bytes Fixed Point with gain code*/
              data[i][j] = 0.0;
              break;
          case 5: /*  IEEE32 4 Bytes Floating*/
              Swap4Char(mm.ch);
              data[i][j] = mm.ft;
              break;
          default:
              data[i][j] = 0.0;
        }
      }
      count += ml+240L;
    }
  }
  *trdata = data;
  return TrNum;
}
//--------------

//-----------------------------------------------------------------------------
void put_segy_text_head(FILE *fp, char str[40][80] )
{
  int i;
  char temchar[80];
  for( i=0;i<40;i++ )
  {
    ascii_to_ebcd(str[i],temchar,80);
    fwrite(temchar,1,80,fp);
  }
  return;
}


//-----------------------------------------------------------------------------
void put_segy_bfile_head(FILE *fp, SEGY_BFILE_HEAD m_Hd)
/*-----------------------------------------------------------------------------------------------------------------------------------------
         Input:
             fp         The pointer to the file writed
             m_Hd    The SEGY binary file head, lies 3200 to 3600 character
        Output:
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int traceh[100],i;
  long count;
  VALUE_TYPE mm;

  for( i=0;i<100;i++ ) traceh[i] = 0;

  count = 3200L;

  mm.lt = m_Hd.NoJob;            Swap4Char(mm.ch);  traceh[0] = mm.it;
  mm.lt = m_Hd.NoLine;          Swap4Char(mm.ch);  traceh[1] = mm.it;
  mm.lt = m_Hd.NoReel;          Swap4Char(mm.ch);  traceh[2] = mm.it;
  mm.st[1] = m_Hd.NumDataTrace;
  mm.st[0] = m_Hd.NumAuxiTrace;      Swap4Char(mm.ch);  traceh[3] = mm.it;
  mm.st[1] = m_Hd.SampleRatioReel;
  mm.st[0] = m_Hd.SampleRatioField;    Swap4Char(mm.ch);  traceh[4] = mm.it;
  mm.st[1] = m_Hd.SampleNumReel;
  mm.st[0] = m_Hd.SampleNumField;      Swap4Char(mm.ch);  traceh[5] = mm.it;
  mm.st[1] = m_Hd.DataFormat;
  mm.st[0] = m_Hd.CDPFold;        Swap4Char(mm.ch);  traceh[6] = mm.it;
  mm.st[1] = m_Hd.TraceSortCode;
  mm.st[0] = m_Hd.VerticalSumCode;    Swap4Char(mm.ch);  traceh[7] = mm.it;
  mm.st[1] = m_Hd.SweepFreqStart;
  mm.st[0] = m_Hd.SweepFreqEnd;      Swap4Char(mm.ch);  traceh[8] = mm.it;
  mm.st[1] = m_Hd.SweepLength;
  mm.st[0] = m_Hd.SweepTypeCode;      Swap4Char(mm.ch);  traceh[9] = mm.it;
  mm.st[1] = m_Hd.SweepTraceNum;
  mm.st[0] = m_Hd.SweepTaperLenStart;    Swap4Char(mm.ch);  traceh[10] = mm.it;
  mm.st[1] = m_Hd.SweepTaperLenEnd;
  mm.st[0] = m_Hd.SweepTaperType;      Swap4Char(mm.ch);  traceh[11] = mm.it;
  mm.st[1] = m_Hd.CorrelDataTrace;
  mm.st[0] = m_Hd.BinaryGainRecover;    Swap4Char(mm.ch);  traceh[12] = mm.it;
  mm.st[1] = m_Hd.AmpRecoverMethod;
  mm.st[0] = m_Hd.MeasurementSys;      Swap4Char(mm.ch);  traceh[13] = mm.it;
  mm.st[1] = m_Hd.ImpulseSignal;
  mm.st[0] = m_Hd.VibratoryPolarityCode;  Swap4Char(mm.ch);  traceh[14] = mm.it;
   mm.st[1] = m_Hd.Revision;
  mm.st[0] = m_Hd.TraceLengthFlag;    Swap4Char(mm.ch);  traceh[75] = mm.it;
  mm.st[0] = 0;
  mm.st[1] = m_Hd.ExtTextNum;        Swap4Char(mm.ch);  traceh[76] = mm.it;

  fseek(fp,count,0);
  fwrite(traceh,4,100,fp);
  return;
}

void put_segy_trace_head(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, SEGY_TRACE_HEAD m_Tr)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp          The pointer to the file readed
        bfh         The SEGY file binary file head
        NO          The trace sequence number in file
        m_Tr        The SEGY trace head struct
     Output:
-----------------------------------------------------------------------------------------------------------------------------------------*/

{
  int traceh[60],i;
  long count;
  long l1,l2,l3,l4,l5,l6,l7,l8;
  int temp;
  VALUE_TYPE mm;

  if( bfh.DataFormat==0 )  return;

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  for( i=0;i<60;i++ ) traceh[i] = 0;

  if( bfh.TraceLengthFlag==1 ){
    count = 3600L+(temp*bfh.SampleNumReel+240L)*(NO-1);
    fseek(fp,count,0);
  }else{
    fseek(fp,0,SEEK_END);
  }

  mm.lt = m_Tr.TraceNoLine;        Swap4Char(mm.ch);  traceh[0] = mm.it;
  mm.lt = m_Tr.TraceNoReel;        Swap4Char(mm.ch);  traceh[1] = mm.it;
  mm.lt = m_Tr.RecordNoField;        Swap4Char(mm.ch);  traceh[2] = mm.it;
  mm.lt = m_Tr.TraceNoField;        Swap4Char(mm.ch);  traceh[3] = mm.it;
  mm.lt = m_Tr.SourceNo;          Swap4Char(mm.ch);  traceh[4] = mm.it;
  mm.lt = m_Tr.CDPNo;            Swap4Char(mm.ch);  traceh[5] = mm.it;
  mm.lt = m_Tr.TraceNoCDP;        Swap4Char(mm.ch);  traceh[6] = mm.it;
  mm.st[1] = m_Tr.TraceCode;
  mm.st[0] = m_Tr.TraceNumVerSum;      Swap4Char(mm.ch);  traceh[7] = mm.it;
  mm.st[1] = m_Tr.TraceNumHorSum;
  mm.st[0] = m_Tr.DataType;        Swap4Char(mm.ch);  traceh[8] = mm.it;
  mm.it = m_Tr.DistanceStoR;        Swap4Char(mm.ch);  traceh[9] = mm.it;

  if( m_Tr.ScalerE>0 ){
    l2 = (long)(m_Tr.ElevReceiver/m_Tr.ScalerE);
    l3 = (long)(m_Tr.ElevSource/m_Tr.ScalerE);
    l4 = (long)(m_Tr.DepthSource/m_Tr.ScalerE);
    l5 = (long)(m_Tr.ElevReceiverDatum/m_Tr.ScalerE);
    l6 = (long)(m_Tr.ElevSourceDatum/m_Tr.ScalerE);
    l7 = (long)(m_Tr.DepthSourceWater/m_Tr.ScalerE);
    l8 = (long)(m_Tr.DepthReceiverWater/m_Tr.ScalerE);
  }else if( m_Tr.ScalerE<0 ){
    l2 = (long)(-m_Tr.ElevReceiver*m_Tr.ScalerE);
    l3 = (long)(-m_Tr.ElevSource*m_Tr.ScalerE);
    l4 = (long)(-m_Tr.DepthSource*m_Tr.ScalerE);
    l5 = (long)(-m_Tr.ElevReceiverDatum*m_Tr.ScalerE);
    l6 = (long)(-m_Tr.ElevSourceDatum*m_Tr.ScalerE);
    l7 = (long)(-m_Tr.DepthSourceWater*m_Tr.ScalerE);
    l8 = (long)(-m_Tr.DepthReceiverWater*m_Tr.ScalerE);
  }else{
    l2 = (long)(m_Tr.ElevReceiver);
    l3 = (long)(m_Tr.ElevSource);
    l4 = (long)(m_Tr.DepthSource);
    l5 = (long)(m_Tr.ElevReceiverDatum);
    l6 = (long)(m_Tr.ElevSourceDatum);
    l7 = (long)(m_Tr.DepthSourceWater);
    l8 = (long)(m_Tr.DepthReceiverWater);
  }

  mm.lt = l2;                Swap4Char(mm.ch);  traceh[10] = mm.it;
  mm.lt = l3;                Swap4Char(mm.ch);  traceh[11] = mm.it;
  mm.lt = l4;                Swap4Char(mm.ch);  traceh[12] = mm.it;
  mm.lt = l5;                Swap4Char(mm.ch);  traceh[13] = mm.it;
  mm.lt = l6;                Swap4Char(mm.ch);  traceh[14] = mm.it;
  mm.lt = l7;                Swap4Char(mm.ch);  traceh[15] = mm.it;
  mm.lt = l8;                Swap4Char(mm.ch);  traceh[16] = mm.it;

  mm.st[1] = m_Tr.ScalerE;
  mm.st[0] = m_Tr.ScalerC;        Swap4Char(mm.ch);  traceh[17] = mm.it;

  if( m_Tr.ScalerC>0 ){
    l1 = (long)(m_Tr.SourceCoordinateX/m_Tr.ScalerC);
    l2 = (long)(m_Tr.SourceCoordinateY/m_Tr.ScalerC);
    l3 = (long)(m_Tr.ReceiverCoordinateX/m_Tr.ScalerC);
    l4 = (long)(m_Tr.ReceiverCoordinateY/m_Tr.ScalerC);
  }else if( m_Tr.ScalerC<0 ){
    l1 = (long)(-m_Tr.SourceCoordinateX*m_Tr.ScalerC);
    l2 = (long)(-m_Tr.SourceCoordinateY*m_Tr.ScalerC);
    l3 = (long)(-m_Tr.ReceiverCoordinateX*m_Tr.ScalerC);
    l4 = (long)(-m_Tr.ReceiverCoordinateY*m_Tr.ScalerC);
  }else{
    l1 = (long)(m_Tr.SourceCoordinateX);
    l2 = (long)(m_Tr.SourceCoordinateY);
    l3 = (long)(m_Tr.ReceiverCoordinateX);
    l4 = (long)(m_Tr.ReceiverCoordinateY);
  }
  mm.lt = l1;                Swap4Char(mm.ch);  traceh[18] = mm.it;
  mm.lt = l2;                Swap4Char(mm.ch);  traceh[19] = mm.it;
  mm.lt = l3;                Swap4Char(mm.ch);  traceh[20] = mm.it;
  mm.lt = l4;                Swap4Char(mm.ch);  traceh[21] = mm.it;

  if( bfh.Revision == 0x0100 ){
    if( m_Tr.ScaleT>0 ){
      m_Tr.UpholeTimeSource /= m_Tr.ScaleT;
      m_Tr.UpholeTimeSource /= m_Tr.ScaleT;
      m_Tr.UpholeTimeReceiver /= m_Tr.ScaleT;
      m_Tr.StaticCorrectSource /= m_Tr.ScaleT;
      m_Tr.StaticCorrectReceiver /= m_Tr.ScaleT;
      m_Tr.TotalStatic /= m_Tr.ScaleT;
      m_Tr.LagTimeA /= m_Tr.ScaleT;
      m_Tr.LagTimeB /= m_Tr.ScaleT;
      m_Tr.DelayTime /= m_Tr.ScaleT;
      m_Tr.MuteTimeStart /= m_Tr.ScaleT;
      m_Tr.MuteTimeEnd /= m_Tr.ScaleT;
    }else if( m_Tr.ScaleT<0 ){
      m_Tr.UpholeTimeSource *= -m_Tr.ScaleT;
      m_Tr.UpholeTimeSource *= -m_Tr.ScaleT;
      m_Tr.UpholeTimeReceiver *= -m_Tr.ScaleT;
      m_Tr.StaticCorrectSource *= -m_Tr.ScaleT;
      m_Tr.StaticCorrectReceiver *= -m_Tr.ScaleT;
      m_Tr.TotalStatic *= -m_Tr.ScaleT;
      m_Tr.LagTimeA *= -m_Tr.ScaleT;
      m_Tr.LagTimeB *= -m_Tr.ScaleT;
      m_Tr.DelayTime *= -m_Tr.ScaleT;
      m_Tr.MuteTimeStart *= -m_Tr.ScaleT;
      m_Tr.MuteTimeEnd *= -m_Tr.ScaleT;
    }else{

    }
  }
  mm.st[1] = m_Tr.CoordinateUnit;
  mm.st[0] = m_Tr.WeatherVelocity;    Swap4Char(mm.ch);  traceh[22] = mm.it;
  mm.st[1] = m_Tr.SubweatherVelocity;
  mm.st[0] = m_Tr.UpholeTimeSource;    Swap4Char(mm.ch);  traceh[23] = mm.it;
  mm.st[1] = m_Tr.UpholeTimeReceiver;
  mm.st[0] = m_Tr.StaticCorrectSource;  Swap4Char(mm.ch);  traceh[24] = mm.it;
  mm.st[1] = m_Tr.StaticCorrectReceiver;
  mm.st[0] = m_Tr.TotalStatic;      Swap4Char(mm.ch);  traceh[25] = mm.it;
  mm.st[1] = m_Tr.LagTimeA;
  mm.st[0] = m_Tr.LagTimeB;        Swap4Char(mm.ch);  traceh[26] = mm.it;
  mm.st[1] = m_Tr.DelayTime;
  mm.st[0] = m_Tr.MuteTimeStart;      Swap4Char(mm.ch);  traceh[27] = mm.it;
  mm.st[1] = m_Tr.MuteTimeEnd;
  mm.st[0] = m_Tr.SampleNum;        Swap4Char(mm.ch);  traceh[28] = mm.it;
  mm.st[1] = m_Tr.SampleInterval;
  mm.st[0] = m_Tr.GainType;        Swap4Char(mm.ch);  traceh[29] = mm.it;
  mm.st[1] = m_Tr.GainConstant;
  mm.st[0] = m_Tr.InitalGain;        Swap4Char(mm.ch);  traceh[30] = mm.it;
  mm.st[1] = m_Tr.Correlated;
  mm.st[0] = m_Tr.SweepFreqStart;      Swap4Char(mm.ch);  traceh[31] = mm.it;
  mm.st[1] = m_Tr.SweepFreqEnd;
  mm.st[0] = m_Tr.SweepLength;      Swap4Char(mm.ch);  traceh[32] = mm.it;
  mm.st[1] = m_Tr.SweepType;
  mm.st[0] = m_Tr.SweepTaperLStart;    Swap4Char(mm.ch);  traceh[33] = mm.it;
  mm.st[1] = m_Tr.SweepTaperLEnd;
  mm.st[0] = m_Tr.SweepTaperType;      Swap4Char(mm.ch);  traceh[34] = mm.it;
  mm.st[1] = m_Tr.AliasFreq;
  mm.st[0] = m_Tr.AliasSlope;        Swap4Char(mm.ch);  traceh[35] = mm.it;
  mm.st[1] = m_Tr.NotchFreq;
  mm.st[0] = m_Tr.NotchSlope;        Swap4Char(mm.ch);  traceh[36] = mm.it;
  mm.st[1] = m_Tr.LowCutFreq;
  mm.st[0] = m_Tr.HighCutFreq;      Swap4Char(mm.ch);  traceh[37] = mm.it;
  mm.st[1] = m_Tr.LowCutSlope;
  mm.st[0] = m_Tr.HighCutSlope;      Swap4Char(mm.ch);  traceh[38] = mm.it;
  mm.st[1] = m_Tr.Year;
  mm.st[0] = m_Tr.Day;          Swap4Char(mm.ch);  traceh[39] = mm.it;
  mm.st[1] = m_Tr.Hour;
  mm.st[0] = m_Tr.Minute;          Swap4Char(mm.ch);  traceh[40] = mm.it;
  mm.st[1] = m_Tr.Second;
  mm.st[0] = m_Tr.TimeBasisCode;      Swap4Char(mm.ch);  traceh[41] = mm.it;
  mm.st[1] = m_Tr.TraceWeightFactor;
  mm.st[0] = m_Tr.GeophoneNoRoll;      Swap4Char(mm.ch);  traceh[42] = mm.it;
  mm.st[1] = m_Tr.GeophoneNoFirstTrace;
  mm.st[0] = m_Tr.GeophoneNoLastTrace;  Swap4Char(mm.ch);  traceh[43] = mm.it;
  mm.st[1] = m_Tr.GapSize;
  mm.st[0] = m_Tr.Overtravel;        Swap4Char(mm.ch);  traceh[44] = mm.it;

  if( bfh.Revision==0x0100 ){
    if( m_Tr.ScalerC>0 ){
      l1 = (long)( m_Tr.CDPPosX/m_Tr.ScalerC );
      l2 = (long)( m_Tr.CDPPosY/m_Tr.ScalerC );
      l3 = (long)( m_Tr.ShotPointNum/m_Tr.ScaleS );
    }else if( m_Tr.ScalerC<0 ){
      l1 = (long)( -m_Tr.CDPPosX*m_Tr.ScalerC );
      l2 = (long)( -m_Tr.CDPPosY*m_Tr.ScalerC );
      l3 = (long)( -m_Tr.ShotPointNum*m_Tr.ScaleS );
    }else{
      l1 = (long)( m_Tr.CDPPosX );
      l2 = (long)( m_Tr.CDPPosY );
      l3 = (long)( m_Tr.ShotPointNum );
    }
    mm.lt = l1;              Swap4Char(mm.ch);  traceh[45] = mm.it;
    mm.lt = l2;              Swap4Char(mm.ch);  traceh[46] = mm.it;
    mm.lt = m_Tr.InLineNo;        Swap4Char(mm.ch);  traceh[47] = mm.it;
    mm.lt = m_Tr.CrossLineNo;      Swap4Char(mm.ch);  traceh[48] = mm.it;

    mm.lt = l3;              Swap4Char(mm.ch);  traceh[49] = mm.it;
    mm.st[1] = m_Tr.ScaleS;
    mm.st[0] = m_Tr.ValueUnit;      Swap4Char(mm.ch);  traceh[50] = mm.it;
/*
    if( m_Tr.TransConstant<32767 ){ l1 = m_Tr.TransConstant; s1 = 0; }
    else if( m_Tr.TransConstant<327670 ){ l1 = m_Tr.TransConstant/10; s1 = 1; }
    else if( m_Tr.TransConstant<3276700 ){l1 = m_Tr.TransConstant/100;s1 = 2; }
    else if( m_Tr.TransConstant<32767000){l1 = m_Tr.TransConstant/1000;s1=3; }
    else if( m_Tr.TransConstant<327670000){l1 = m_Tr.TransConstant/10000; s1 =4; }
    else if( m_Tr.TransConstant<3276700000){l1 = m_Tr.TransConstant/100000;s1 = 5;}
    else{  l1 = m_Tr.TransConstant/1000000; s1 = 6;}
*/
    mm.lt = m_Tr.TransConstantB;    Swap4Char(mm.ch);  traceh[51] = mm.it;
    mm.st[1] = m_Tr.TransConstantE;
    mm.st[0] = m_Tr.TransUnit;      Swap4Char(mm.ch);  traceh[52] = mm.it;
    mm.st[1] = m_Tr.DeviceID;
    mm.st[0] = m_Tr.ScaleT;        Swap4Char(mm.ch);  traceh[53] = mm.it;
  }
  fwrite(traceh,4,60,fp);
  return;
}


void put_segy_trace_data(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, float *data)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp          The file pointer
        bfh         The SEGY file binary file head
        NO          The trace sequence number in file
        data        The saved data
     Output:
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int i,temp;
  long count;
  float *tr;
  VALUE_TYPE mm;

  if( bfh.DataFormat==0 )  return;

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  if( bfh.TraceLengthFlag==1 ){
    count = 3840L+(temp*bfh.SampleNumReel+240L)*(NO-1);
    fseek(fp,count,0);
  }else{
    fseek(fp,0,SEEK_END);
 }

  tr = (float*)malloc(temp*bfh.SampleNumReel);

  for( i=0;i<bfh.SampleNumReel;i++ ){
    mm.ft = data[i];
    switch( bfh.DataFormat ){
      case 1:  /* IBM 4 Bytes Floating*/
          mm.ft = float_to_ibm(mm.ft);
          break;
      case 2:  /* IBM 4 Bytes Fixed Point*/
          break;
      case 3:  /* IBM 2 Bytes Fixed Point*/
          break;
      case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
          break;
      case 5:  /* IEEE32 4 Bytes Floating*/
          Swap4Char(mm.ch);
          break;
      default:
          ;
    }
    tr[i] = mm.ft;
  }
  fwrite(tr,temp,bfh.SampleNumReel,fp);
  free(tr);
  return;
}

void put_array_in_segy(FILE *fp, int format,int TrNum, int num, float dx, float dt, float **data)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp          The file pointer
        format      The SEGY file format
        TrNum       The Number of trace
        num         The number of sample each trace if len_flag = 1.
        dx          The distance of trace  in m
        dt          The sample ratio in ms
        NO          The trace sequence number in file
     Output:
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int i;
  char m_EBCD[40][80];
  SEGY_BFILE_HEAD m_Hd;
  SEGY_TRACE_HEAD m_Tr;

/*                             1         2         3         4         5         6         7         8
                      12345678901234567890123456789012345678901234567890123456789012345678901234567890         */
  strcpy(m_EBCD[0],"C 1 CLIENT                        COMPANY  KEYIN Ltd. Co.       CREW NO         ");
  strcpy(m_EBCD[1],"C 2 LINE            AREA                                                        ");
  strcpy(m_EBCD[2],"C 3 REEL NO           DAY-START OF REEL     YEAR      OBSERVER                  ");
  strcpy(m_EBCD[3],"C 4 INSTRUMENT  MFG            MODEL            SERIAL NO                       ");
  strcpy(m_EBCD[4],"C 5 DATA TRACES/RECORD        AUXILIARY TRACES/RECORD         CDP FOLD          ");
  strcpy(m_EBCD[5],"C 6 SAMPLE INTERVAL         SAMPLES/TRACE       BITS/IN      BYTES/SAMPLE       ");
  strcpy(m_EBCD[6],"C 7 RECORDING FORMAT        FORMAT THIS REEL SEG-Y  MEASUREMENT SYSTEM METERS   ");
  strcpy(m_EBCD[7],"C 8 SAMPLE CODE FLOATING PT                                                     ");
  strcpy(m_EBCD[8],"C 9 GAIN  TYPE                                                                  ");
  strcpy(m_EBCD[9],"C10 FILTERS                                                                     ");
  strcpy(m_EBCD[10],"C11 SOURCE  TYPE            NUMBER/POINT        POINT INTERVAL                  ");
  strcpy(m_EBCD[11],"C12     PATTERN                            LENGTH        WIDTH                  ");
  strcpy(m_EBCD[12],"C13 SWEEP  START     HZ  END     HZ  LENGTH      MS  CHANNEL NO     TYPE        ");
  strcpy(m_EBCD[13],"C14 TAPER  START LENGTH       MS  END LENGTH       MS  TYPE                     ");
  strcpy(m_EBCD[14],"C15 SPREAD  OFFSET        MAX DISTANCE        GROUP INTERVAL                    ");
  strcpy(m_EBCD[15],"C16 GEOPHONES  PER GROUP     SPACING     FREQUENCY     MFG          MODEL       ");
  strcpy(m_EBCD[16],"C17      TYPE                              LENGTH        WIDTH                  ");
  strcpy(m_EBCD[17],"C18 TRACES SORTED BY RECORD        PROJECT                LINE ID               ");
  strcpy(m_EBCD[18],"C19 AMPLITUDE RECOVERY                                                          ");
  strcpy(m_EBCD[19],"C20 MAP PROJECTION                      ZONE ID       COORDINATE UNITS          ");
  strcpy(m_EBCD[20],"C21 FIELD SUM       NAVIGATION SYSTEM               RECORDING PARTY             ");
  strcpy(m_EBCD[21],"C22 CABLE TYPE                   DEPTH        SHOOTING DIRECTION                ");
  strcpy(m_EBCD[22],"C23 PROCES SIGN:                                                                ");
  strcpy(m_EBCD[23],"C24 PROCES SIGN:                                                                ");
  strcpy(m_EBCD[24],"C25                                                                             ");
  strcpy(m_EBCD[25],"C26                                                                             ");
  strcpy(m_EBCD[26],"C27                                                                             ");
  strcpy(m_EBCD[27],"C28                                                                             ");
  strcpy(m_EBCD[28],"C29                                                                             ");
  strcpy(m_EBCD[29],"C30                                                                             ");
  strcpy(m_EBCD[30],"C31                                                                             ");
  strcpy(m_EBCD[31],"C32                                                                             ");
  strcpy(m_EBCD[32],"C33                                                                             ");
  strcpy(m_EBCD[33],"C34                                                                             ");
  strcpy(m_EBCD[34],"C35                                                                             ");
  strcpy(m_EBCD[35],"C36                                                                             ");
  strcpy(m_EBCD[36],"C37                                                                             ");
  strcpy(m_EBCD[37],"C38                                                                             ");
  strcpy(m_EBCD[38],"C39 SEG Y rev0                                                                 ");
  strcpy(m_EBCD[39],"C40 END EBCDIC                                                                  ");

  m_Hd.NoJob = 100;
  m_Hd.NoLine = 1;
  m_Hd.NoReel = 1;
  m_Hd.NumDataTrace = (short)TrNum;
  m_Hd.NumAuxiTrace = 0;
  m_Hd.SampleRatioReel = (short)(dt*1000);
  m_Hd.SampleRatioField = (short)(dt*1000);
  m_Hd.SampleNumReel = (short)num;
  m_Hd.SampleNumField = (short)num;
  m_Hd.DataFormat = 5;
  m_Hd.CDPFold = 1;
  m_Hd.TraceSortCode = 1;
  m_Hd.VerticalSumCode = 1;
  m_Hd.SweepFreqStart = 0;
  m_Hd.SweepFreqEnd = 0;
  m_Hd.SweepLength = 0;
  m_Hd.SweepTypeCode = 0;
  m_Hd.SweepTraceNum = 0;
  m_Hd.SweepTaperLenStart = 0;
  m_Hd.SweepTaperLenEnd = 0;
  m_Hd.SweepTaperType = 0;
   m_Hd.CorrelDataTrace = 0;
  m_Hd.BinaryGainRecover = 0;
  m_Hd.AmpRecoverMethod = 0;
  m_Hd.MeasurementSys = 0;
  m_Hd.ImpulseSignal = 0;
  m_Hd.VibratoryPolarityCode = 0;
  m_Hd.Revision = 0;
  m_Hd.TraceLengthFlag = 1;
  m_Hd.ExtTextNum = 0;

  m_Tr.TraceNumVerSum = 1;
  m_Tr.TraceNumHorSum = 1;
  m_Tr.DataType = 1;
  m_Tr.DistanceStoR = 0.0f;
  m_Tr.ElevReceiver = 0.0f;
  m_Tr.ElevSource = 0.0f;
  m_Tr.DepthSource = 0.0f;
  m_Tr.ElevReceiverDatum = 0.0f;
  m_Tr.ElevSourceDatum = 0.0f;
  m_Tr.DepthSourceWater = 0.0f;
  m_Tr.DepthReceiverWater = 0.0f;
  m_Tr.ScalerE = 1;
  m_Tr.ScalerC = 1;
  m_Tr.SourceCoordinateY = 0.0f;
  m_Tr.ReceiverCoordinateY = 0.0f;
  m_Tr.CoordinateUnit = 1;
  m_Tr.WeatherVelocity = 0;
  m_Tr.SubweatherVelocity = 0;
  m_Tr.UpholeTimeSource = 0;
  m_Tr.UpholeTimeReceiver = 0;
  m_Tr.StaticCorrectSource = 0;
  m_Tr.StaticCorrectReceiver = 0;
  m_Tr.TotalStatic = 0;
  m_Tr.LagTimeA = 0;
  m_Tr.LagTimeB = 0;
  m_Tr.DelayTime = 0;
  m_Tr.MuteTimeStart = 0;
  m_Tr.MuteTimeEnd = 0;
  m_Tr.SampleNum = (short)num;
  m_Tr.SampleInterval = (short)(dt*1000);
  m_Tr.GainType = 0;
  m_Tr.GainConstant = 0;
  m_Tr.InitalGain = 0;
  m_Tr.Correlated = 0;
  m_Tr.SweepFreqStart = 0;
  m_Tr.SweepFreqEnd = 0;
  m_Tr.SweepLength = 0;
  m_Tr.SweepType = 0;
  m_Tr.SweepTaperLStart = 0;
  m_Tr.SweepTaperLEnd = 0;
  m_Tr.SweepTaperType = 0;
  m_Tr.AliasFreq = 0;
  m_Tr.AliasSlope = 0;
  m_Tr.NotchFreq = 0;
  m_Tr.NotchSlope = 0;
  m_Tr.LowCutFreq = 0;
  m_Tr.HighCutFreq = 0;
  m_Tr.LowCutSlope = 0;
  m_Tr.HighCutSlope = 0;
  m_Tr.Year = 0;
  m_Tr.Day = 0;
  m_Tr.Hour = 0;
  m_Tr.Minute = 0;
  m_Tr.Second = 0;
  m_Tr.TimeBasisCode = 0;
  m_Tr.TraceWeightFactor = 0;
  m_Tr.GeophoneNoRoll = 0;
  m_Tr.GeophoneNoFirstTrace = 0;
  m_Tr.GeophoneNoLastTrace = 0;
  m_Tr.GapSize = 0;
  m_Tr.Overtravel = 0;
  m_Tr.CDPPosX = 0;
  m_Tr.CDPPosY = 0;
  m_Tr.InLineNo = 0;
  m_Tr.CrossLineNo = 0;
  m_Tr.ShotPointNum = 0;
  m_Tr.ScaleS = 0;
  m_Tr.ValueUnit = 0;
  m_Tr.TransConstantB = 0;
  m_Tr.TransConstantE = 0;
  m_Tr.TransUnit = 0;
  m_Tr.DeviceID = 0;
  m_Tr.ScaleT = 0;

  put_segy_text_head(fp, m_EBCD);
  put_segy_bfile_head(fp, m_Hd);

  for( i=0;i<TrNum;i++ ){
    m_Tr.TraceNoLine = i+1;
    m_Tr.TraceNoReel = i+1;
    m_Tr.RecordNoField = i+1;
    m_Tr.TraceNoField = i+1;
    m_Tr.CDPNo = i+1;
    m_Tr.SourceNo = i+1;
    m_Tr.TraceNoCDP = i+1;
    m_Tr.TraceCode = 1;
    m_Tr.SourceCoordinateX = (i+1)*dx;
    m_Tr.ReceiverCoordinateX = (i+1)*dx;
    put_segy_trace_head(fp, m_Hd, i+1, m_Tr);
    put_segy_trace_data(fp, m_Hd, i+1, data[i]);
  }

  return;
}



void print_segy_text_head(FILE *fp, char m_EBCD[40][80])
{
  int i,j;

  fprintf(fp,"\n/*************************** SEGY text file head ******************************/\n");
  for( i=0;i<40;i++ ){
    for( j=0;j<80;j++){
      fprintf(fp, "%c",m_EBCD[i][j]);
      if( (j+1)%80 == 0 ) fprintf(fp, "\n");
    }
  }

  return;

}


void print_segy_bfile_head(FILE *fp, SEGY_BFILE_HEAD m_Hd)
/*-----------------------------------------------------------------------------------------------------------------------------------------
         Input:
             fp         The pointer to the file writed
             m_Hd    The SEGY binary file head, lies 3200 to 3600 character
        Output:
-----------------------------------------------------------------------------------------------------------------------------------------*/
{

  fprintf(fp,"\n/*************************** SEGY binary file head ****************************/\n");
  fprintf(fp,"                     Job Identification Numbers : %ld\n", m_Hd.NoJob);
  fprintf(fp,"                                    Line Number : %ld\n", m_Hd.NoLine);
  fprintf(fp,"                      Binary Header Reel Number : %ld\n", m_Hd.NoReel);
  fprintf(fp,"               Number of Data Traces per Record : %d\n", m_Hd.NumDataTrace);
  fprintf(fp,"               Number of Aux. Traces per Record : %d\n", m_Hd.NumAuxiTrace);
  fprintf(fp,"      Sample Rate in microseconds for this Reel : %d\n", m_Hd.SampleRatioReel);
  fprintf(fp,"Sample Rate in microseconds for Field Recording : %d\n", m_Hd.SampleRatioField);
  fprintf(fp,"       Number of Samples per Trace for this Reel: %d\n", m_Hd.SampleNumReel);
  fprintf(fp," Number of Samples per Trace for Field Recording: %d\n", m_Hd.SampleNumField);
  fprintf(fp,"             Data Sample Format (1-ibm, 5-ieee) : %d\n", m_Hd.DataFormat);
  fprintf(fp,"                                       CDP Fold : %d\n", m_Hd.CDPFold);
  fprintf(fp,"                             Trace Sorting Code : %d\n", m_Hd.TraceSortCode);
  fprintf(fp,"                              Vertical Sum Code : %d\n", m_Hd.VerticalSumCode);
  fprintf(fp,"                       Sweep Frequency At Start : %d\n", m_Hd.SweepFreqStart);
  fprintf(fp,"                         Sweep Frequency At End : %d\n", m_Hd.SweepFreqEnd);
  fprintf(fp,"                               Sweep Length(ms) : %d\n", m_Hd.SweepLength);
  fprintf(fp,"                                Sweep Type Code : %d\n", m_Hd.SweepTypeCode);
  fprintf(fp,"                  Trace Number of Sweep Channel : %d\n", m_Hd.SweepTraceNum);
  fprintf(fp,"               Sweep Trace Taper in ms at start : %d\n", m_Hd.SweepTaperLenStart);
  fprintf(fp,"                 Sweep Trace Taper in ms at end : %d\n", m_Hd.SweepTaperLenEnd);
  fprintf(fp,"                                     Taper Type : %d\n", m_Hd.SweepTaperType);
  fprintf(fp,"                         Correlated Data Traces : %d\n", m_Hd.CorrelDataTrace);
  fprintf(fp,"                          Binary Gain Recovered : %d\n", m_Hd.CorrelDataTrace);
  fprintf(fp,"                      Amplitude Recovery Method : %d\n", m_Hd.AmpRecoverMethod);
  fprintf(fp,"                       Units (1-meters, 2-feet) : %d\n", m_Hd.MeasurementSys);
  fprintf(fp,"                                 Impulse Signal : %d\n", m_Hd.ImpulseSignal);
  fprintf(fp,"                        Vibratory Polarity Code : %d\n", m_Hd.VibratoryPolarityCode);

  fprintf(fp,"                   SEY-Y Format Revision Number : %d\n", m_Hd.Revision);
  fprintf(fp,"                        Fixed length trace flag : %d\n", m_Hd.TraceLengthFlag);
  fprintf(fp,"   Number of 3200-byte Textual Header Extension : %d\n", m_Hd.ExtTextNum);

  fprintf(fp,"\n");

}

void print_segy_trace_head(FILE *fp,int NO, SEGY_TRACE_HEAD m_Tr)
/*-----------------------------------------------------------------------------------------------------------------------------------------
         Input:
             fp         The pointer to the file writed
             m_Tr    The SEGY trace head, lies 001 to 240 character  in each trace
        Output:
-----------------------------------------------------------------------------------------------------------------------------------------*/{
  fprintf(fp,"\n");
  fprintf(fp," ============== The trace head  of %4dth Trace =============\n",NO);
  fprintf(fp,"              Trace Sequence Number Within Line : %ld\n",m_Tr.TraceNoLine);
  fprintf(fp,"              Trace Sequence Number Within Reel : %ld\n",m_Tr.TraceNoReel);
  fprintf(fp,"                   Original Field Record Number : %ld\n",m_Tr.RecordNoField);
  fprintf(fp,"  Trace Number Within the Original Field Record : %ld\n",m_Tr.TraceNoField);
  fprintf(fp,"                     Energy Source Point Number : %ld\n",m_Tr.SourceNo);
  fprintf(fp,"                            CDP Ensemble Number : %ld\n",m_Tr.CDPNo);
  fprintf(fp,"           Trace Number Within the CDP Ensemble : %ld\n",m_Tr.TraceNoCDP);

  fprintf(fp,"                      Trace Identification Code : %d\n",m_Tr.TraceCode);
  fprintf(fp,"             Number of Vertically Summed Traces : %d\n",m_Tr.TraceNumVerSum);
  fprintf(fp,"          Number of Horizzontally Summed Traces : %d\n",m_Tr.TraceNumHorSum);
  fprintf(fp,"                                      Data Type : %d\n",m_Tr.DataType);

  fprintf(fp,"   Distance From Source Point To Receiver Group : %d\n",m_Tr.DistanceStoR);
  fprintf(fp,"                       Receiver Group Elevation : %-10.2f\n",m_Tr.ElevReceiver);
  fprintf(fp,"                    Surface Elevation At Source : %-10.2f\n",m_Tr.ElevSource);
  fprintf(fp,"                     Source Depth Below Surface : %-10.2f\n",m_Tr.DepthSource);
  fprintf(fp,"             Dataum Elevation At Receiver Group : %-10.2f\n",m_Tr.ElevReceiverDatum);
  fprintf(fp,"                     Dataum Elevation At Source : %-10.2f\n",m_Tr.ElevSourceDatum);
  fprintf(fp,"                          Water Depth At Source : %-10.2f\n",m_Tr.DepthSourceWater);
  fprintf(fp,"                           Water Depth at Group : %-10.2f\n",m_Tr.DepthReceiverWater);

  fprintf(fp,"Scaler To Be Applied To All Elevations & Depths : %d\n",m_Tr.ScalerE);
  fprintf(fp," Scaler To Be Used To All Coordinates Specified : %d\n",m_Tr.ScalerC);

  fprintf(fp,"                            Source CoordinateX : %-10.2f\n",m_Tr.SourceCoordinateX);
  fprintf(fp,"                            Source CoordinateY : %-10.2f\n",m_Tr.SourceCoordinateY);
  fprintf(fp,"                             Group CoordinateX : %-10.2f\n",m_Tr.ReceiverCoordinateX);
  fprintf(fp,"                             Group CoordinateY : %-10.2f\n",m_Tr.ReceiverCoordinateY);

  fprintf(fp,"                              Coordinates Units : %d\n",m_Tr.CoordinateUnit);
  fprintf(fp,"                            Weathering Velocity : %d\n",m_Tr.WeatherVelocity);
  fprintf(fp,"                         Subweathering Velocity : %d\n",m_Tr.SubweatherVelocity);
  fprintf(fp,"                          Uphole Time at Source : %d\n",m_Tr.UpholeTimeSource);
  fprintf(fp,"                           Uphole Time at Group : %d\n",m_Tr.UpholeTimeReceiver);
  fprintf(fp,"                       Source Static Correction : %d\n",m_Tr.StaticCorrectSource);
  fprintf(fp,"                        Group Static Correction : %d\n",m_Tr.StaticCorrectReceiver);
  fprintf(fp,"                           Total Static Applied : %d\n",m_Tr.TotalStatic);
  fprintf(fp,"                                     Lag Time A : %d\n",m_Tr.LagTimeA);
  fprintf(fp,"                                     Lag Time B : %d\n",m_Tr.LagTimeB);
  fprintf(fp,"                                Delay Recording : %d\n",m_Tr.DelayTime);
  fprintf(fp,"                                Mute Time-Start : %d\n",m_Tr.MuteTimeStart);
  fprintf(fp,"                                  Mute Time-End : %d\n",m_Tr.MuteTimeEnd);
  fprintf(fp,"                Number of Samples in this Trace : %d\n",m_Tr.SampleNum);
  fprintf(fp,"                          Sample Interval in ms : %d\n",m_Tr.SampleInterval);
  fprintf(fp,"                 Gain Type of Field Instruments : %d\n",m_Tr.GainType);
  fprintf(fp,"                       Instrument Gain Constant : %d\n",m_Tr.GainConstant);
  fprintf(fp,"           Instrument Early or Initial Gain(Db) : %d\n",m_Tr.InitalGain);
  fprintf(fp,"                                     Correlated : %d\n",m_Tr.Correlated);
  fprintf(fp,"                       Sweep Frequency at Start : %d\n",m_Tr.SweepFreqStart);
  fprintf(fp,"                         Sweep Frequency at End : %d\n",m_Tr.SweepFreqEnd);
  fprintf(fp,"                             Sweep Length in ms : %d\n",m_Tr.SweepLength);
  fprintf(fp,"                                     Sweep Type : %d\n",m_Tr.SweepType);
  fprintf(fp,"              Sweep Trace Taper Length at Start : %d\n",m_Tr.SweepTaperLStart);
  fprintf(fp,"                Sweep Trace Taper Length at End : %d\n",m_Tr.SweepTaperLEnd);
  fprintf(fp,"                                     Taper Type : %d\n",m_Tr.SweepTaperType);
  fprintf(fp,"                 Alias Filter Frequency if Used : %d\n",m_Tr.AliasFreq);
  fprintf(fp,"                             Alias Filter Slope : %d\n",m_Tr.AliasSlope);
  fprintf(fp,"                 Notch Filter Frequency if used : %d\n",m_Tr.NotchFreq);
  fprintf(fp,"                             Notch Filter Slope : %d\n",m_Tr.NotchSlope);
  fprintf(fp,"                      Low Cut Frequency if used : %d\n",m_Tr.LowCutFreq);
  fprintf(fp,"                     High Cut Frequency if used : %d\n",m_Tr.HighCutFreq);
  fprintf(fp,"                                  Low Cut Slope : %d\n",m_Tr.LowCutSlope);
  fprintf(fp,"                                 High Cut Slope : %d\n",m_Tr.HighCutSlope);
  fprintf(fp,"                             Year data Recorded : %d\n",m_Tr.Year);
  fprintf(fp,"                                    Day of Year : %d\n",m_Tr.Day);
  fprintf(fp,"                                    Hour of Day : %d\n",m_Tr.Hour);
  fprintf(fp,"                                 Minute of Hour : %d\n",m_Tr.Minute);
  fprintf(fp,"                               Second of Minute : %d\n",m_Tr.Second);
  fprintf(fp,"                                Time Basis Code : %d\n",m_Tr.TimeBasisCode);
  fprintf(fp,"                         Trace Weighting Factor : %d\n",m_Tr.TraceWeightFactor);
  fprintf(fp,"Geophone Group Number of Roll Switch Position 1 : %d\n",m_Tr.GeophoneNoRoll);
  fprintf(fp,"             Geophone Group Number of Trace one : %d\n",m_Tr.GeophoneNoFirstTrace);
  fprintf(fp,"            Geophone Group Number of last Trace : %d\n",m_Tr.GeophoneNoLastTrace);
  fprintf(fp,"                                       Gap Size : %d\n",m_Tr.GapSize);
  fprintf(fp,"               OverTravel Associated with Taper : %d\n",m_Tr.Overtravel);

    fprintf(fp,"                   X coordinate of CDP position : %-10.2f\n",m_Tr.CDPPosX);
    fprintf(fp,"                   Y coordinate of CDP position : %-10.2f\n",m_Tr.CDPPosY);
    fprintf(fp,"                                 In-line number : %ld\n",m_Tr.InLineNo);
    fprintf(fp,"                              Cross-line number : %ld\n",m_Tr.CrossLineNo);
    fprintf(fp,"                               Shotpoint number : %-10.2f\n",m_Tr.ShotPointNum);
    fprintf(fp,"   Scalar to be applied to the shotpoint number : %d\n",m_Tr.ScaleS);
    fprintf(fp,"                   Trace value measurement unit : %d\n",m_Tr.ValueUnit);
    fprintf(fp,"                          Transduction Constant : %ld\n",m_Tr.TransConstantB);
    fprintf(fp,"                             Transduction Units : %d\n",m_Tr.TransUnit);
    fprintf(fp,"                        Device/Trace Identifier : %d\n",m_Tr.DeviceID);
    fprintf(fp,"Scalar to be applied to times in bytes 95-114 to\n");
    fprintf(fp,"                           give true time in ms : %d\n",m_Tr.ScaleT);

  return;
}

void print_trace_data(FILE *fp, int TrNum, int num, float **data)
{
  int i,j;
  fprintf(fp, "Trace  ");
  for( i=1;i<=TrNum;i++ ) fprintf(fp, "%12d",i);
  fprintf(fp,"\n");
  for( i=0;i<num;i++ ){
    fprintf(fp, "%5d   ",i+1);
    for( j=0;j<TrNum;j++ ){
      fprintf(fp, "%12.3f", data[j][i]);
    }
    fprintf(fp,"\n");
  }
}


int get_segy_trace_num(FILE *fp, SEGY_BFILE_HEAD bfh)
{
  int move,file_len,ml;
  int trnum,temp;

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  ml = temp*bfh.SampleNumReel+240l;
  move = 3600+bfh.ExtTextNum*3200l;

 if( bfh.TraceLengthFlag==1 ){
    file_len = get_file_length( fp );
    trnum = (int)( (file_len-move)/ml );
  }else{

    trnum = 0;

  }

  return trnum;

}


void get_segy_cdp_no(FILE *fp, SEGY_BFILE_HEAD bfh, int **cdpno)
{
  int   i,temp,trnum;
  int *cdp;
  long  move,ml;
  VALUE_TYPE mm;

  if( *cdpno!=NULL ) Free1Int(*cdpno);

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  ml = temp*bfh.SampleNumReel+240l;
  move = 3600l+bfh.ExtTextNum*3200l;

  if( bfh.TraceLengthFlag==1 ){

    trnum = get_segy_trace_num(fp, bfh);
//    fprintf(stderr, "Trace Number : %d\n",trnum);
    cdp = Alloc1Int(trnum);

    for( i=0;i<trnum;i++ ){
      fseek(fp,move+20, SEEK_SET);
      fread(&mm.lt,4,1,fp);         Swap4Char(mm.ch);  cdp[i] = mm.it;
      move += ml;
     // printf("   i= %d cdp[i]= %d\n",i,cdp[i]);
    }
  }else{
    cdp = NULL;
    fprintf(stderr,"Can't process variable trace length at present!\n");
  }

  *cdpno = cdp;

  return;
}



void get_segy_trace_data1(FILE *fp,unsigned short Rev, int format,int len_flag,int num, int NO, float *data)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp            The pointer to the file readed
        Rev         The SEGY file revision number
        format      The SEGY file format
        len_flag    The Fixed length trace flag.
                       =1 indicates to have the same number of samples,
                       =0 indicates that the length of the traces in the file may vary
        num         The number of sample each trace if len_flag = 1.
        NO          The trace sequence number in file
     Output:
        data        The SEGY trace data
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int i;
  long count,temp;
  VALUE_TYPE mm;

  if( format==0 )  return;

  if( format==3 ) temp = 2;
  else if( format==8 ) temp = 1;
  else temp = 4;

  if( len_flag==1 ){
    count = 3840L+(temp*num+240L)*(NO-1);
  }else{
    return;
  }

  fseek(fp,count,0);
  fread(data,temp,num,fp);
  for( i=0;i<num;i++ ){
    mm.ft = data[i];
    switch( format ){
      case 1:  /*IBM 4 Bytes Floating*/
          data[i] = ibm_to_float(mm.ft);
          break;
      case 2:  /* IBM 4 Bytes Fixed Point*/
          data[i] = 0.0;
          break;
      case 3:  /* IBM 2 Bytes Fixed Point*/
          data[i] = 0.0;
          break;
      case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
          data[i] = 0.0;
          break;
      case 5:  /* IEEE32 4 Bytes Floating*/
          Swap4Char(mm.ch);
          data[i] = (float)(mm.ft);
          break;
      default:
          data[i] = 0.0;
    }
  }

  return;
}
void put_segy_trace_data1(FILE *fp, unsigned short Rev, int format,int len_flag,int num, int NO, float *data)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp          The file pointer
        Rev         The SEGY file revision number
        format      The SEGY file format
        len_flag    The Fixed length trace flag.
                       =1 indicates to have the same number of samples,
                       =0 indicates that the length of the traces in the file may vary
        num         The number of sample each trace if len_flag = 1.
        NO          The trace sequence number in file
     Output:
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int i,temp;
  long count;
  VALUE_TYPE mm;

  if( format==0 )  return;

  if( format==3 ) temp = 2;
  else if( format==8 ) temp = 1;
  else temp = 4;

/*  fprintf(stderr,"000000 ok,  num:%d\n",num);*/

  if( len_flag==1 ){
    count = 3840L+(temp*num+240L)*(NO-1);
    fseek(fp,count,0);
  }else{
    fseek(fp,0,SEEK_END);
  }
/*
  tr = Alloc1Float(num);
  if( tr==NULL ) {fprintf(stderr,"3333333 ok\n"); return;}
  fprintf(stderr,"111111 ok\n");
*/
  for( i=0;i<num;i++ ){
    mm.ft = data[i];
    switch( format ){
      case 1:  /* IBM 4 Bytes Floating*/
          mm.ft = float_to_ibm(mm.ft);
          break;
      case 2:  /* IBM 4 Bytes Fixed Point*/
          break;
      case 3:  /* IBM 2 Bytes Fixed Point*/
          break;
      case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
          break;
      case 5:  /* IEEE32 4 Bytes Floating*/
          Swap4Char(mm.ch);
          break;
      default:
          ;
    }
    data[i] = mm.ft;
  }
  fwrite(data,temp,num,fp);
/*  Free1Float(tr);*/
  return;
}


void get_segy_trace(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, SEGY_TRACE_HEAD *m_Tr, float *tr)
/*-----------------------------------------------------------------------------------------------------------------------------------------
    Input:
        fp          The pointer to the file readed
        bfh         The SEGY file binary file head
        NO          The trace sequence number in file
     Output:
        m_Tr        The SEGY trace head of the NO trace
        tr          The SEGY trace data of the NO trace
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int traceh[60];
  long count;
  int temp,num, i;
  VALUE_TYPE mm;

  if( bfh.DataFormat==0 )  return;

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  if( bfh.TraceLengthFlag==1 ){
    num = bfh.SampleNumReel;
    count = 3600L+(temp*num+240L)*(NO-1);
  }else{
    return;
  }

  fseek(fp,count,0);
  fread(traceh,4,60,fp);
  mm.it = traceh[0];  Swap4Char(mm.ch);   m_Tr->TraceNoLine = mm.it;
  mm.it = traceh[1];  Swap4Char(mm.ch);   m_Tr->TraceNoReel = mm.it;
  mm.it = traceh[2];  Swap4Char(mm.ch);   m_Tr->RecordNoField = mm.it;
  mm.it = traceh[3];  Swap4Char(mm.ch);   m_Tr->TraceNoField = mm.it;
  mm.it = traceh[4];  Swap4Char(mm.ch);   m_Tr->SourceNo = mm.it;
  mm.it = traceh[5];  Swap4Char(mm.ch);   m_Tr->CDPNo = mm.it;
  mm.it = traceh[6];  Swap4Char(mm.ch);   m_Tr->TraceNoCDP = mm.it;
  mm.it = traceh[7];  Swap4Char(mm.ch);   m_Tr->TraceCode = mm.st[1];        m_Tr->TraceNumVerSum = mm.st[0];
  mm.it = traceh[8];  Swap4Char(mm.ch);   m_Tr->TraceNumHorSum = mm.st[1];   m_Tr->DataType = mm.st[0];
  mm.it = traceh[9];  Swap4Char(mm.ch);   m_Tr->DistanceStoR = mm.it;

  mm.it = traceh[17]; Swap4Char(mm.ch);   m_Tr->ScalerE = mm.st[1];           m_Tr->ScalerC = mm.st[0];
  if( m_Tr->ScalerE>0 ){
    mm.it = traceh[10]; Swap4Char(mm.ch);   m_Tr->ElevReceiver = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[11]; Swap4Char(mm.ch);   m_Tr->ElevSource = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[12]; Swap4Char(mm.ch);   m_Tr->DepthSource = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[13]; Swap4Char(mm.ch);   m_Tr->ElevReceiverDatum = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[14]; Swap4Char(mm.ch);   m_Tr->ElevSourceDatum = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[15]; Swap4Char(mm.ch);   m_Tr->DepthSourceWater = (float)mm.lt*m_Tr->ScalerE;
    mm.it = traceh[16]; Swap4Char(mm.ch);   m_Tr->DepthReceiverWater = (float)mm.lt*m_Tr->ScalerE;
  }else if( m_Tr->ScalerE<0 ){
    mm.it = traceh[10]; Swap4Char(mm.ch);   m_Tr->ElevReceiver = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[11]; Swap4Char(mm.ch);   m_Tr->ElevSource = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[12]; Swap4Char(mm.ch);   m_Tr->DepthSource = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[13]; Swap4Char(mm.ch);   m_Tr->ElevReceiverDatum = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[14]; Swap4Char(mm.ch);   m_Tr->ElevSourceDatum = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[15]; Swap4Char(mm.ch);   m_Tr->DepthSourceWater = (float)mm.lt/m_Tr->ScalerE;
    mm.it = traceh[16]; Swap4Char(mm.ch);   m_Tr->DepthReceiverWater = (float)mm.lt/m_Tr->ScalerE;
  }else{
    mm.it = traceh[10]; Swap4Char(mm.ch);   m_Tr->ElevReceiver = (float)mm.it;
    mm.it = traceh[11]; Swap4Char(mm.ch);   m_Tr->ElevSource = (float)mm.it;
    mm.it = traceh[12]; Swap4Char(mm.ch);   m_Tr->DepthSource = (float)mm.it;
    mm.it = traceh[13]; Swap4Char(mm.ch);   m_Tr->ElevReceiverDatum = (float)mm.it;
    mm.it = traceh[14]; Swap4Char(mm.ch);   m_Tr->ElevSourceDatum = (float)mm.it;
    mm.it = traceh[15]; Swap4Char(mm.ch);   m_Tr->DepthSourceWater = (float)mm.it;
    mm.it = traceh[16]; Swap4Char(mm.ch);   m_Tr->DepthReceiverWater = (float)mm.it;
  }

  if( m_Tr->ScalerC>0 ){
    mm.it = traceh[18]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateX = (float)mm.lt*m_Tr->ScalerC;
    mm.it = traceh[19]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateY = (float)mm.lt*m_Tr->ScalerC;
    mm.it = traceh[20]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateX = (float)mm.lt*m_Tr->ScalerC;
    mm.it = traceh[21]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateY = (float)mm.lt*m_Tr->ScalerC;
  }else if( m_Tr->ScalerC<0 ){
    mm.it = traceh[18]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateX = (float)mm.lt/m_Tr->ScalerC;
    mm.it = traceh[19]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateY = (float)mm.lt/m_Tr->ScalerC;
    mm.it = traceh[20]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateX = (float)mm.lt/m_Tr->ScalerC;
    mm.it = traceh[21]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateY = (float)mm.lt/m_Tr->ScalerC;
  }else{
    mm.it = traceh[18]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateX = (float)mm.it;
    mm.it = traceh[19]; Swap4Char(mm.ch);   m_Tr->SourceCoordinateY = (float)mm.it;
    mm.it = traceh[20]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateX = (float)mm.it;
    mm.it = traceh[21]; Swap4Char(mm.ch);   m_Tr->ReceiverCoordinateY = (float)mm.it;
  }

  mm.it = traceh[22]; Swap4Char(mm.ch);   m_Tr->CoordinateUnit = mm.st[1];        m_Tr->WeatherVelocity = mm.st[0];
  mm.it = traceh[23]; Swap4Char(mm.ch);   m_Tr->SubweatherVelocity = mm.st[1];    m_Tr->UpholeTimeSource = mm.st[0];
  mm.it = traceh[24]; Swap4Char(mm.ch);   m_Tr->UpholeTimeReceiver = mm.st[1];    m_Tr->StaticCorrectSource = mm.st[0];
  mm.it = traceh[25]; Swap4Char(mm.ch);   m_Tr->StaticCorrectReceiver = mm.st[1]; m_Tr->TotalStatic = mm.st[0];
  mm.it = traceh[26]; Swap4Char(mm.ch);   m_Tr->LagTimeA = mm.st[1];              m_Tr->LagTimeB = mm.st[0];
  mm.it = traceh[27]; Swap4Char(mm.ch);   m_Tr->DelayTime = mm.st[1];             m_Tr->MuteTimeStart = mm.st[0];
  mm.it = traceh[28]; Swap4Char(mm.ch);   m_Tr->MuteTimeEnd = mm.st[1];           m_Tr->SampleNum = mm.st[0];
  mm.it = traceh[29]; Swap4Char(mm.ch);   m_Tr->SampleInterval = mm.st[1];        m_Tr->GainType = mm.st[0];
  mm.it = traceh[30]; Swap4Char(mm.ch);   m_Tr->GainConstant = mm.st[1];          m_Tr->InitalGain = mm.st[0];
  mm.it = traceh[31]; Swap4Char(mm.ch);   m_Tr->Correlated = mm.st[1];            m_Tr->SweepFreqStart = mm.st[0];
  mm.it = traceh[32]; Swap4Char(mm.ch);   m_Tr->SweepFreqEnd = mm.st[1];          m_Tr->SweepLength = mm.st[0];
  mm.it = traceh[33]; Swap4Char(mm.ch);   m_Tr->SweepType = mm.st[1];             m_Tr->SweepTaperLStart = mm.st[0];
  mm.it = traceh[34]; Swap4Char(mm.ch);   m_Tr->SweepTaperLEnd = mm.st[1];        m_Tr->SweepTaperType = mm.st[0];
  mm.it = traceh[35]; Swap4Char(mm.ch);   m_Tr->AliasFreq = mm.st[1];             m_Tr->AliasSlope = mm.st[0];
  mm.it = traceh[36]; Swap4Char(mm.ch);   m_Tr->NotchFreq = mm.st[1];             m_Tr->NotchSlope = mm.st[0];
  mm.it = traceh[37]; Swap4Char(mm.ch);   m_Tr->LowCutFreq = mm.st[1];            m_Tr->HighCutFreq = mm.st[0];
  mm.it = traceh[38]; Swap4Char(mm.ch);   m_Tr->LowCutSlope = mm.st[1];           m_Tr->HighCutSlope = mm.st[0];
  mm.it = traceh[39]; Swap4Char(mm.ch);   m_Tr->Year = mm.st[1];                  m_Tr->Day = mm.st[0];
  mm.it = traceh[40]; Swap4Char(mm.ch);   m_Tr->Hour = mm.st[1];                  m_Tr->Minute = mm.st[0];
  mm.it = traceh[41]; Swap4Char(mm.ch);   m_Tr->Second = mm.st[1];                m_Tr->TimeBasisCode = mm.st[0];
  mm.it = traceh[42]; Swap4Char(mm.ch);   m_Tr->TraceWeightFactor = mm.st[1];     m_Tr->GeophoneNoRoll = mm.st[0];
  mm.it = traceh[43]; Swap4Char(mm.ch);   m_Tr->GeophoneNoFirstTrace = mm.st[1];  m_Tr->GeophoneNoLastTrace = mm.st[0];
  mm.it = traceh[44]; Swap4Char(mm.ch);   m_Tr->GapSize = mm.st[1];               m_Tr->Overtravel = mm.st[0];
  mm.it = traceh[45]; Swap4Char(mm.ch);   m_Tr->CDPPosX = (float)mm.lt;
  mm.it = traceh[46]; Swap4Char(mm.ch);   m_Tr->CDPPosY = (float)mm.lt;
  mm.it = traceh[47]; Swap4Char(mm.ch);   m_Tr->InLineNo = mm.lt;
  mm.it = traceh[48]; Swap4Char(mm.ch);   m_Tr->CrossLineNo = mm.lt;
  mm.it = traceh[49]; Swap4Char(mm.ch);   m_Tr->ShotPointNum = mm.ft;
  mm.it = traceh[50]; Swap4Char(mm.ch);   m_Tr->ScaleS = mm.st[1];                m_Tr->ValueUnit = mm.st[0];
  mm.it = traceh[51]; Swap4Char(mm.ch);   m_Tr->TransConstantB = mm.lt;
  mm.it = traceh[52]; Swap4Char(mm.ch);   m_Tr->TransConstantE = mm.st[1];        m_Tr->TransUnit = mm.st[0];
  mm.it = traceh[53]; Swap4Char(mm.ch);   m_Tr->DeviceID = mm.st[1];              m_Tr->ScaleT = mm.st[0];
  m_Tr->Reserved[0] = traceh[54];
  m_Tr->Reserved[1] = traceh[55];
  m_Tr->Reserved[2] = traceh[56];
  m_Tr->Reserved[3] = traceh[57];
  m_Tr->Reserved[4] = traceh[58];
  m_Tr->Reserved[5] = traceh[59];

  if( bfh.Revision==0x0100 ){
    if( m_Tr->ScaleT>0 ){
      m_Tr->CDPPosX *= m_Tr->ScalerC;
      m_Tr->CDPPosY *= m_Tr->ScalerC;
      m_Tr->ShotPointNum *= m_Tr->ScalerC;
      m_Tr->UpholeTimeSource *= m_Tr->ScaleT;
      m_Tr->UpholeTimeSource *= m_Tr->ScaleT;
      m_Tr->UpholeTimeReceiver *= m_Tr->ScaleT;
      m_Tr->StaticCorrectSource *= m_Tr->ScaleT;
      m_Tr->StaticCorrectReceiver *= m_Tr->ScaleT;
      m_Tr->TotalStatic *= m_Tr->ScaleT;
      m_Tr->LagTimeA *= m_Tr->ScaleT;
      m_Tr->LagTimeB *= m_Tr->ScaleT;
      m_Tr->DelayTime *= m_Tr->ScaleT;
      m_Tr->MuteTimeStart *= m_Tr->ScaleT;
      m_Tr->MuteTimeEnd *= m_Tr->ScaleT;
    }else if( m_Tr->ScaleT<0 ){
      m_Tr->CDPPosX /= -m_Tr->ScalerC;
      m_Tr->CDPPosY /= -m_Tr->ScalerC;
      m_Tr->ShotPointNum /= -m_Tr->ScalerC;
      m_Tr->UpholeTimeSource /= -m_Tr->ScaleT;
      m_Tr->UpholeTimeSource /= -m_Tr->ScaleT;
      m_Tr->UpholeTimeReceiver /= -m_Tr->ScaleT;
      m_Tr->StaticCorrectSource /= -m_Tr->ScaleT;
      m_Tr->StaticCorrectReceiver /= -m_Tr->ScaleT;
      m_Tr->TotalStatic /= -m_Tr->ScaleT;
      m_Tr->LagTimeA /= -m_Tr->ScaleT;
      m_Tr->LagTimeB /= -m_Tr->ScaleT;
      m_Tr->DelayTime /= -m_Tr->ScaleT;
      m_Tr->MuteTimeStart /= -m_Tr->ScaleT;
      m_Tr->MuteTimeEnd /= -m_Tr->ScaleT;
    }else{

    }
  }

  fseek(fp,count+240,0);
  fread(tr,temp,num,fp);
  for( i=0;i<num;i++ ){
    mm.ft = tr[i];
    switch( bfh.DataFormat ){
      case 1:  /*IBM 4 Bytes Floating*/
          tr[i] = ibm_to_float(mm.ft);
          break;
      case 2:  /* IBM 4 Bytes Fixed Point*/
          tr[i] = 0.0;
          break;
      case 3:  /* IBM 2 Bytes Fixed Point*/
          tr[i] = 0.0;
          break;
      case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
          tr[i] = 0.0;
          break;
      case 5:  /* IEEE32 4 Bytes Floating*/
          Swap4Char(mm.ch);
          tr[i] = (float)(mm.ft);
          break;
      default:
          tr[i] = 0.0;
    }
  }

  return;
}

void put_segy_trace(FILE *fp, SEGY_BFILE_HEAD bfh, int NO, SEGY_TRACE_HEAD m_Tr,  float *tr)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp          The pointer to the file readed
        bfh         The SEGY file binary file head
        NO          The trace sequence number in file
        m_Tr        The SEGY trace head struct
        tr          The SEGY trace data
     Output:
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int traceh[60],i;
  long count;
  long l1,l2,l3,l4,l5,l6,l7,l8;
  int temp;
  VALUE_TYPE mm;

  if( bfh.DataFormat==0 )  return;

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  for( i=0;i<60;i++ ) traceh[i] = 0;

  if( bfh.TraceLengthFlag==1 ){
    count = 3600L+(temp*bfh.SampleNumReel+240L)*(NO-1);
    fseek(fp,count,0);
  }else{
    fseek(fp,0,SEEK_END);
  }

  mm.lt = m_Tr.TraceNoLine;        Swap4Char(mm.ch);  traceh[0] = mm.it;
  mm.lt = m_Tr.TraceNoReel;        Swap4Char(mm.ch);  traceh[1] = mm.it;
  mm.lt = m_Tr.RecordNoField;        Swap4Char(mm.ch);  traceh[2] = mm.it;
  mm.lt = m_Tr.TraceNoField;        Swap4Char(mm.ch);  traceh[3] = mm.it;
  mm.lt = m_Tr.SourceNo;          Swap4Char(mm.ch);  traceh[4] = mm.it;
  mm.lt = m_Tr.CDPNo;            Swap4Char(mm.ch);  traceh[5] = mm.it;
  mm.lt = m_Tr.TraceNoCDP;        Swap4Char(mm.ch);  traceh[6] = mm.it;
  mm.st[1] = m_Tr.TraceCode;
  mm.st[0] = m_Tr.TraceNumVerSum;      Swap4Char(mm.ch);  traceh[7] = mm.it;
  mm.st[1] = m_Tr.TraceNumHorSum;
  mm.st[0] = m_Tr.DataType;        Swap4Char(mm.ch);  traceh[8] = mm.it;
  mm.it = m_Tr.DistanceStoR;        Swap4Char(mm.ch);  traceh[9] = mm.it;

  if( m_Tr.ScalerE>0 ){
    l2 = (long)(m_Tr.ElevReceiver/m_Tr.ScalerE);
    l3 = (long)(m_Tr.ElevSource/m_Tr.ScalerE);
    l4 = (long)(m_Tr.DepthSource/m_Tr.ScalerE);
    l5 = (long)(m_Tr.ElevReceiverDatum/m_Tr.ScalerE);
    l6 = (long)(m_Tr.ElevSourceDatum/m_Tr.ScalerE);
    l7 = (long)(m_Tr.DepthSourceWater/m_Tr.ScalerE);
    l8 = (long)(m_Tr.DepthReceiverWater/m_Tr.ScalerE);
  }else if( m_Tr.ScalerE<0 ){
    l2 = (long)(-m_Tr.ElevReceiver*m_Tr.ScalerE);
    l3 = (long)(-m_Tr.ElevSource*m_Tr.ScalerE);
    l4 = (long)(-m_Tr.DepthSource*m_Tr.ScalerE);
    l5 = (long)(-m_Tr.ElevReceiverDatum*m_Tr.ScalerE);
    l6 = (long)(-m_Tr.ElevSourceDatum*m_Tr.ScalerE);
    l7 = (long)(-m_Tr.DepthSourceWater*m_Tr.ScalerE);
    l8 = (long)(-m_Tr.DepthReceiverWater*m_Tr.ScalerE);
  }else{
    l2 = (long)(m_Tr.ElevReceiver);
    l3 = (long)(m_Tr.ElevSource);
    l4 = (long)(m_Tr.DepthSource);
    l5 = (long)(m_Tr.ElevReceiverDatum);
    l6 = (long)(m_Tr.ElevSourceDatum);
    l7 = (long)(m_Tr.DepthSourceWater);
    l8 = (long)(m_Tr.DepthReceiverWater);
  }

  mm.lt = l2;                Swap4Char(mm.ch);  traceh[10] = mm.it;
  mm.lt = l3;                Swap4Char(mm.ch);  traceh[11] = mm.it;
  mm.lt = l4;                Swap4Char(mm.ch);  traceh[12] = mm.it;
  mm.lt = l5;                Swap4Char(mm.ch);  traceh[13] = mm.it;
  mm.lt = l6;                Swap4Char(mm.ch);  traceh[14] = mm.it;
  mm.lt = l7;                Swap4Char(mm.ch);  traceh[15] = mm.it;
  mm.lt = l8;                Swap4Char(mm.ch);  traceh[16] = mm.it;

  mm.st[1] = m_Tr.ScalerE;
  mm.st[0] = m_Tr.ScalerC;        Swap4Char(mm.ch);  traceh[17] = mm.it;

  if( m_Tr.ScalerC>0 ){
    l1 = (long)(m_Tr.SourceCoordinateX/m_Tr.ScalerC);
    l2 = (long)(m_Tr.SourceCoordinateY/m_Tr.ScalerC);
    l3 = (long)(m_Tr.ReceiverCoordinateX/m_Tr.ScalerC);
    l4 = (long)(m_Tr.ReceiverCoordinateY/m_Tr.ScalerC);
  }else if( m_Tr.ScalerC<0 ){
    l1 = (long)(-m_Tr.SourceCoordinateX*m_Tr.ScalerC);
    l2 = (long)(-m_Tr.SourceCoordinateY*m_Tr.ScalerC);
    l3 = (long)(-m_Tr.ReceiverCoordinateX*m_Tr.ScalerC);
    l4 = (long)(-m_Tr.ReceiverCoordinateY*m_Tr.ScalerC);
  }else{
    l1 = (long)(m_Tr.SourceCoordinateX);
    l2 = (long)(m_Tr.SourceCoordinateY);
    l3 = (long)(m_Tr.ReceiverCoordinateX);
    l4 = (long)(m_Tr.ReceiverCoordinateY);
  }
  mm.lt = l1;                Swap4Char(mm.ch);  traceh[18] = mm.it;
  mm.lt = l2;                Swap4Char(mm.ch);  traceh[19] = mm.it;
  mm.lt = l3;                Swap4Char(mm.ch);  traceh[20] = mm.it;
  mm.lt = l4;                Swap4Char(mm.ch);  traceh[21] = mm.it;

  if( bfh.Revision == 0x0100 ){
    if( m_Tr.ScaleT>0 ){
      m_Tr.UpholeTimeSource /= m_Tr.ScaleT;
      m_Tr.UpholeTimeSource /= m_Tr.ScaleT;
      m_Tr.UpholeTimeReceiver /= m_Tr.ScaleT;
      m_Tr.StaticCorrectSource /= m_Tr.ScaleT;
      m_Tr.StaticCorrectReceiver /= m_Tr.ScaleT;
      m_Tr.TotalStatic /= m_Tr.ScaleT;
      m_Tr.LagTimeA /= m_Tr.ScaleT;
      m_Tr.LagTimeB /= m_Tr.ScaleT;
      m_Tr.DelayTime /= m_Tr.ScaleT;
      m_Tr.MuteTimeStart /= m_Tr.ScaleT;
      m_Tr.MuteTimeEnd /= m_Tr.ScaleT;
    }else if( m_Tr.ScaleT<0 ){
      m_Tr.UpholeTimeSource *= -m_Tr.ScaleT;
      m_Tr.UpholeTimeSource *= -m_Tr.ScaleT;
      m_Tr.UpholeTimeReceiver *= -m_Tr.ScaleT;
      m_Tr.StaticCorrectSource *= -m_Tr.ScaleT;
      m_Tr.StaticCorrectReceiver *= -m_Tr.ScaleT;
      m_Tr.TotalStatic *= -m_Tr.ScaleT;
      m_Tr.LagTimeA *= -m_Tr.ScaleT;
      m_Tr.LagTimeB *= -m_Tr.ScaleT;
      m_Tr.DelayTime *= -m_Tr.ScaleT;
      m_Tr.MuteTimeStart *= -m_Tr.ScaleT;
      m_Tr.MuteTimeEnd *= -m_Tr.ScaleT;
    }else{

    }
  }
  mm.st[1] = m_Tr.CoordinateUnit;
  mm.st[0] = m_Tr.WeatherVelocity;    Swap4Char(mm.ch);  traceh[22] = mm.it;
  mm.st[1] = m_Tr.SubweatherVelocity;
  mm.st[0] = m_Tr.UpholeTimeSource;    Swap4Char(mm.ch);  traceh[23] = mm.it;
  mm.st[1] = m_Tr.UpholeTimeReceiver;
  mm.st[0] = m_Tr.StaticCorrectSource;  Swap4Char(mm.ch);  traceh[24] = mm.it;
  mm.st[1] = m_Tr.StaticCorrectReceiver;
  mm.st[0] = m_Tr.TotalStatic;      Swap4Char(mm.ch);  traceh[25] = mm.it;
  mm.st[1] = m_Tr.LagTimeA;
  mm.st[0] = m_Tr.LagTimeB;        Swap4Char(mm.ch);  traceh[26] = mm.it;
  mm.st[1] = m_Tr.DelayTime;
  mm.st[0] = m_Tr.MuteTimeStart;      Swap4Char(mm.ch);  traceh[27] = mm.it;
  mm.st[1] = m_Tr.MuteTimeEnd;
  mm.st[0] = m_Tr.SampleNum;        Swap4Char(mm.ch);  traceh[28] = mm.it;
  mm.st[1] = m_Tr.SampleInterval;
  mm.st[0] = m_Tr.GainType;        Swap4Char(mm.ch);  traceh[29] = mm.it;
  mm.st[1] = m_Tr.GainConstant;
  mm.st[0] = m_Tr.InitalGain;        Swap4Char(mm.ch);  traceh[30] = mm.it;
  mm.st[1] = m_Tr.Correlated;
  mm.st[0] = m_Tr.SweepFreqStart;      Swap4Char(mm.ch);  traceh[31] = mm.it;
  mm.st[1] = m_Tr.SweepFreqEnd;
  mm.st[0] = m_Tr.SweepLength;      Swap4Char(mm.ch);  traceh[32] = mm.it;
  mm.st[1] = m_Tr.SweepType;
  mm.st[0] = m_Tr.SweepTaperLStart;    Swap4Char(mm.ch);  traceh[33] = mm.it;
  mm.st[1] = m_Tr.SweepTaperLEnd;
  mm.st[0] = m_Tr.SweepTaperType;      Swap4Char(mm.ch);  traceh[34] = mm.it;
  mm.st[1] = m_Tr.AliasFreq;
  mm.st[0] = m_Tr.AliasSlope;        Swap4Char(mm.ch);  traceh[35] = mm.it;
  mm.st[1] = m_Tr.NotchFreq;
  mm.st[0] = m_Tr.NotchSlope;        Swap4Char(mm.ch);  traceh[36] = mm.it;
  mm.st[1] = m_Tr.LowCutFreq;
  mm.st[0] = m_Tr.HighCutFreq;      Swap4Char(mm.ch);  traceh[37] = mm.it;
  mm.st[1] = m_Tr.LowCutSlope;
  mm.st[0] = m_Tr.HighCutSlope;      Swap4Char(mm.ch);  traceh[38] = mm.it;
  mm.st[1] = m_Tr.Year;
  mm.st[0] = m_Tr.Day;          Swap4Char(mm.ch);  traceh[39] = mm.it;
  mm.st[1] = m_Tr.Hour;
  mm.st[0] = m_Tr.Minute;          Swap4Char(mm.ch);  traceh[40] = mm.it;
  mm.st[1] = m_Tr.Second;
  mm.st[0] = m_Tr.TimeBasisCode;      Swap4Char(mm.ch);  traceh[41] = mm.it;
  mm.st[1] = m_Tr.TraceWeightFactor;
  mm.st[0] = m_Tr.GeophoneNoRoll;      Swap4Char(mm.ch);  traceh[42] = mm.it;
  mm.st[1] = m_Tr.GeophoneNoFirstTrace;
  mm.st[0] = m_Tr.GeophoneNoLastTrace;  Swap4Char(mm.ch);  traceh[43] = mm.it;
  mm.st[1] = m_Tr.GapSize;
  mm.st[0] = m_Tr.Overtravel;        Swap4Char(mm.ch);  traceh[44] = mm.it;

  if( bfh.Revision==0x0100 ){
    if( m_Tr.ScalerC>0 ){
      l1 = (long)( m_Tr.CDPPosX/m_Tr.ScalerC );
      l2 = (long)( m_Tr.CDPPosY/m_Tr.ScalerC );
      l3 = (long)( m_Tr.ShotPointNum/m_Tr.ScaleS );
    }else if( m_Tr.ScalerC<0 ){
      l1 = (long)( -m_Tr.CDPPosX*m_Tr.ScalerC );
      l2 = (long)( -m_Tr.CDPPosY*m_Tr.ScalerC );
      l3 = (long)( -m_Tr.ShotPointNum*m_Tr.ScaleS );
    }else{
      l1 = (long)( m_Tr.CDPPosX );
      l2 = (long)( m_Tr.CDPPosY );
      l3 = (long)( m_Tr.ShotPointNum );
    }
    mm.lt = l1;              Swap4Char(mm.ch);  traceh[45] = mm.it;
    mm.lt = l2;              Swap4Char(mm.ch);  traceh[46] = mm.it;
    mm.lt = m_Tr.InLineNo;        Swap4Char(mm.ch);  traceh[47] = mm.it;
    mm.lt = m_Tr.CrossLineNo;      Swap4Char(mm.ch);  traceh[48] = mm.it;

    mm.lt = l3;              Swap4Char(mm.ch);  traceh[49] = mm.it;
    mm.st[1] = m_Tr.ScaleS;
    mm.st[0] = m_Tr.ValueUnit;      Swap4Char(mm.ch);  traceh[50] = mm.it;
/*
    if( m_Tr.TransConstant<32767 ){ l1 = m_Tr.TransConstant; s1 = 0; }
    else if( m_Tr.TransConstant<327670 ){ l1 = m_Tr.TransConstant/10; s1 = 1; }
    else if( m_Tr.TransConstant<3276700 ){l1 = m_Tr.TransConstant/100;s1 = 2; }
    else if( m_Tr.TransConstant<32767000){l1 = m_Tr.TransConstant/1000;s1=3; }
    else if( m_Tr.TransConstant<327670000){l1 = m_Tr.TransConstant/10000; s1 =4; }
    else if( m_Tr.TransConstant<3276700000){l1 = m_Tr.TransConstant/100000;s1 = 5;}
    else{  l1 = m_Tr.TransConstant/1000000; s1 = 6;}
*/
    mm.lt = m_Tr.TransConstantB;    Swap4Char(mm.ch);  traceh[51] = mm.it;
    mm.st[1] = m_Tr.TransConstantE;
    mm.st[0] = m_Tr.TransUnit;      Swap4Char(mm.ch);  traceh[52] = mm.it;
    mm.st[1] = m_Tr.DeviceID;
    mm.st[0] = m_Tr.ScaleT;        Swap4Char(mm.ch);  traceh[53] = mm.it;
  }
  fwrite(traceh,4,60,fp);

  for( i=0;i<bfh.SampleNumReel;i++ ){
    mm.ft = tr[i];
    switch( bfh.DataFormat ){
      case 1:  /* IBM 4 Bytes Floating*/
          mm.ft = float_to_ibm(mm.ft);
          break;
      case 2:  /* IBM 4 Bytes Fixed Point*/
          break;
      case 3:  /* IBM 2 Bytes Fixed Point*/
          break;
      case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
          break;
      case 5:  /* IEEE32 4 Bytes Floating*/
          Swap4Char(mm.ch);
          break;
      default:
          ;
    }
    tr[i] = mm.ft;
  }
  fwrite(tr,temp,bfh.SampleNumReel,fp);

  return;
}


void get_segy_trace_all(FILE *fp, SEGY_BFILE_HEAD bfh, SEGY_TRACE_HEAD *tr, int FirTr, int EndTr, float **data)
{
  int i;

  if( bfh.DataFormat==0 )  return;
  if( FirTr<=0 ) FirTr = 1;
  if( EndTr<FirTr ) EndTr = get_segy_trace_num(fp, bfh);

  for( i=0;i<EndTr-FirTr+1;i++ ){
    get_segy_trace(fp, bfh, FirTr+i, &tr[i], data[i]);
/*    fprintf(stdout,"%d  ",FirTr+i);
    tr[i] = trh;*/
  }

  return;
}

void put_segy_trace_all(FILE *fp, SEGY_BFILE_HEAD bfh, SEGY_TRACE_HEAD *tr, int FirTr, int EndTr, float **data)
{
  int i;

  if( bfh.DataFormat==0 )  return;
  if( FirTr<=0 ) FirTr = 1;
  if( EndTr<FirTr ) EndTr = get_segy_trace_num(fp, bfh);

  for( i=0;i<EndTr-FirTr+1;i++ ){
    put_segy_trace(fp, bfh, FirTr+i, tr[i], data[i]);
  }

  return;

}

int check_segy_trace_len(const char *filename)
{

  return 1;
}


int get_segy_sorce_XY(FILE *fp, SEGY_BFILE_HEAD bfh, float ***XY)
{
  int i;
  int trnum;
  float **temp;
  SEGY_TRACE_HEAD trh;
  
  trnum = get_segy_trace_num(fp, bfh);
  temp = Alloc2Float(3, trnum);
  
  for( i=0;i<trnum;i++ ){
    get_segy_trace_head(fp, bfh, i+1, &trh);
    temp[0][i] = (float)trh.CDPNo;
    temp[1][i] = trh.ReceiverCoordinateX;  
    temp[2][i] = trh.ReceiverCoordinateY;
  }
  
  *XY = temp; 
  return trnum; 
}




/*********************************************************************************************/


void put_trace(FILE *fp, float *tr, int num, int NO)
{
  long move;
  move = (NO-1)*num*4;
  fseek(fp, move, 0);
  fwrite(tr,4,num,fp);
}


void get_trace(FILE *fp, float *tr, int num, int NO)
{
  long move;
  move = (NO-1)*num*4;
  fseek(fp, move, 0);
  fread(tr,4,num,fp);
}

void put_line(FILE *fp, float **data, int TrNum, int num, int line)
{
  int i;
  long move;

  move = sizeof(float)*TrNum*num*(line-1);
  fseek(fp, move, SEEK_SET);
  for( i=0;i<TrNum;i++ )  fwrite(data[i], sizeof(float), num, fp);

  return;
}

void get_line(FILE *fp, float **data, int TrNum, int num, int line)
{
  int i;
  long move;

  move = sizeof(float)*TrNum*num*(line-1);
  fseek(fp, move, SEEK_SET);
  for( i=0;i<TrNum;i++ )  fread(data[i], sizeof(float), num, fp);

  return;
}


int get_su_trace_head(FILE *fp, segy *tr,int NO,int nl)
{
 VALUE_TYPE mm;
  int trh[60];
  int count;
  count = (nl*sizeof(float)+240L)*(NO-1);
  fseek(fp,count,SEEK_SET);
  if(!fread(trh,sizeof(int),60,fp)) return 0;

  mm.it = trh[0]; Swap4Char(mm.ch);  tr->tracl = mm.it;
  mm.it = trh[1]; Swap4Char(mm.ch);  tr->tracr = mm.it;
  mm.it = trh[2]; Swap4Char(mm.ch);  tr->fldr = mm.it;
  mm.it = trh[3]; Swap4Char(mm.ch);   tr->tracf = mm.it;
  mm.it = trh[4]; Swap4Char(mm.ch);   tr->ep = mm.it;
  mm.it = trh[5]; Swap4Char(mm.ch);   tr->cdp = mm.it;
  mm.it = trh[6]; Swap4Char(mm.ch);   tr->cdpt = mm.it;

  mm.it = trh[7]; Swap4Char(mm.ch);  tr->trid = mm.st[1];tr->nvs = mm.st[0];
  mm.it = trh[8]; Swap4Char(mm.ch);  tr->nhs = mm.st[1];tr->duse = mm.st[0];

  mm.it = trh[9]; Swap4Char(mm.ch);  tr->offset = mm.it;
  mm.it = trh[10]; Swap4Char(mm.ch);  tr->gelev = mm.it;
  mm.it = trh[11]; Swap4Char(mm.ch);  tr->selev = mm.it;
  mm.it = trh[12]; Swap4Char(mm.ch);   tr->sdepth = mm.it;
  mm.it = trh[13]; Swap4Char(mm.ch);   tr->gdel = mm.it;
  mm.it = trh[14]; Swap4Char(mm.ch);   tr->sdel = mm.it;
  mm.it = trh[15]; Swap4Char(mm.ch);   tr->swdep = mm.it;
  mm.it = trh[16]; Swap4Char(mm.ch);   tr->gwdep = mm.it;

  mm.it = trh[17]; Swap4Char(mm.ch);  tr->scalel = mm.st[1];tr->scalco = mm.st[0];
  mm.it = trh[18]; Swap4Char(mm.ch);  tr->sx = mm.it;
  mm.it = trh[19]; Swap4Char(mm.ch);  tr->sy = mm.it;
  mm.it = trh[20]; Swap4Char(mm.ch);  tr->gx = mm.it;
  mm.it = trh[21]; Swap4Char(mm.ch);  tr->gy = mm.it;
  mm.it = trh[22]; Swap4Char(mm.ch);  tr->counit = mm.st[1];tr->wevel = mm.st[0];
  mm.it = trh[23]; Swap4Char(mm.ch);  tr->swevel = mm.st[1];tr->sut = mm.st[0];
  mm.it = trh[24]; Swap4Char(mm.ch);  tr->gut = mm.st[1];tr->sstat = mm.st[0];
  mm.it = trh[25]; Swap4Char(mm.ch);  tr->gstat = mm.st[1];tr->tstat = mm.st[0];
  mm.it = trh[26]; Swap4Char(mm.ch);  tr->laga = mm.st[1];tr->lagb = mm.st[0];
  mm.it = trh[27]; Swap4Char(mm.ch);  tr->delrt = mm.st[1]; tr->muts = mm.st[0];
  mm.it = trh[28]; Swap4Char(mm.ch);  tr->mute = mm.st[1];tr->ns = mm.st[0];
  mm.it = trh[29]; Swap4Char(mm.ch);  tr->dt = mm.st[1]; tr->gain = mm.st[0];

  mm.it = trh[30]; Swap4Char(mm.ch);  tr->igc = mm.st[1];tr->igi = mm.st[0];
  mm.it = trh[31]; Swap4Char(mm.ch);  tr->corr = mm.st[1];tr->sfs = mm.st[0];
  mm.it = trh[32]; Swap4Char(mm.ch);  tr->sfe = mm.st[1];tr->slen = mm.st[0];
  mm.it = trh[33]; Swap4Char(mm.ch);  tr->styp = mm.st[1];tr->stas = mm.st[0];
  mm.it = trh[34]; Swap4Char(mm.ch);  tr->stae = mm.st[1];tr->tatyp = mm.st[0];
  mm.it = trh[35]; Swap4Char(mm.ch);  tr->afilf = mm.st[1]; tr->afils = mm.st[0];
  mm.it = trh[36]; Swap4Char(mm.ch);  tr->nofilf = mm.st[1];tr->nofils = mm.st[0];
  mm.it = trh[37]; Swap4Char(mm.ch);  tr->lcf = mm.st[1]; tr->hcf = mm.st[0];
  mm.it = trh[38]; Swap4Char(mm.ch);  tr->lcs = mm.st[1];tr->hcs = mm.st[0];
  mm.it = trh[39]; Swap4Char(mm.ch);  tr->year = mm.st[1]; tr->day = mm.st[0];
  mm.it = trh[40]; Swap4Char(mm.ch);  tr->hour = mm.st[1];tr->minute = mm.st[0];
  mm.it = trh[41]; Swap4Char(mm.ch);  tr->sec = mm.st[1]; tr->timbas = mm.st[0];


  mm.it = trh[42]; Swap4Char(mm.ch);   tr->trwf = mm.st[1]; tr->grnors = mm.st[0];
  mm.it = trh[43]; Swap4Char(mm.ch);   tr->grnofr = mm.st[1]; tr->grnlof = mm.st[0];

  mm.it = trh[44]; Swap4Char(mm.ch);   tr->gaps = mm.st[1]; tr->otrav = mm.st[0];

  mm.it = trh[45]; Swap4Char(mm.ch);    tr->d1 = mm.ft;
  mm.it = trh[46]; Swap4Char(mm.ch);    tr->f1 = mm.ft;
  mm.it = trh[47]; Swap4Char(mm.ch);    tr->d2 = mm.ft;
  mm.it = trh[48]; Swap4Char(mm.ch);    tr->f2 = mm.ft;
  mm.it = trh[49]; Swap4Char(mm.ch);    tr->ungpow = mm.ft;

  mm.it = trh[50]; Swap4Char(mm.ch);   tr->unscale = mm.ft;
  mm.it = trh[51]; Swap4Char(mm.ch);   tr->ntr = mm.it;
  mm.it = trh[52]; Swap4Char(mm.ch);   tr->mark = mm.st[1];   tr->mutb = mm.st[0];
  mm.it = trh[53]; Swap4Char(mm.ch);   tr->dz = mm.ft;
  mm.it = trh[54]; Swap4Char(mm.ch);   tr->fz = mm.ft;
  mm.it = trh[55]; Swap4Char(mm.ch);   tr->n2 = mm.st[1]; tr->shortpad = mm.st[0];

  mm.it = trh[56]; Swap4Char(mm.ch);
     tr->unass[0] = (float)mm.st[1];   tr->unass[1] = (float)mm.st[0];
  mm.it = trh[57]; Swap4Char(mm.ch);
     tr->unass[2] = (float)mm.st[1];   
  return 1;
}

int get_segy_cdpdata(int fp, SEGY_BFILE_HEAD bfh, int firstcdp, int endcdp, float ***trdata)
/*-----------------------------------------------------------------------------------------------------------------------------------------
      Get all data in the same CDP from opened file.
        If NO<1, get the data in the first CDP.  If NO>maxcdpnum in file,get the data in the last CDP.
-------------------------------------------------------------------------------------------------------------------------------------------
        Input:
             fp      The pointer to the file read
             bfh     The SEGY binary file head, lies 3200 to 3600 character
         firstcdp    The first cdp number in file
          endcdp     The last cdp number in file
        Output:
             trdata  The data in same cdp trace
        Return:
             trnum   The number of trace in the cdp
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int   i,j,temp,trnum, trnum0, cdpnum;
  long  tmcdp,ml;
  __int64 move,move0,filelen;
  float **data;
  VALUE_TYPE mm;

  if( firstcdp>endcdp ) endcdp = firstcdp;
  if( endcdp<=0 ) endcdp = 100000;  /* read all cdp data */

//  if( bfh.DataFormat==3 ) temp = 2;
  //else if( bfh.DataFormat==8 ) temp = 1;
//  else temp = 4;

  filelen=(__int64)filelength(fp);

  if(filelen<0) filelen=(long)filelen;
  tmcdp = 0;

  ml = temp*bfh.SampleNumReel+240;
  move = 3600l+bfh.ExtTextNum*3200l;
  move0 = move+240;

  if( *trdata!=NULL ){ Free2Float(*trdata); *trdata=NULL; }

  trnum = 0;
  trnum0 = 0;
  cdpnum = 0;

  if( bfh.TraceLengthFlag==1 ){

   /* Count the number of trace in same CDP */
   for( ;;) {
      if((FileSeek(fp, move+20, 0))==-1) break;
      FileRead(fp,&mm.lt,4);         Swap4Char(mm.ch);  tmcdp = mm.it;
/*      fprintf(stderr," The CDP number is %ld in the file\n",tmcdp);  */
      if( tmcdp<firstcdp ){
        trnum0++;
        move += ml;
        if( move>=filelen ) break;
        continue;
      }else if( tmcdp>endcdp ){
        break;
      }else{
        trnum++;
        move += ml;
        if( move>=filelen ) break;
      }
    }

    if( trnum==0 ){
      fprintf(stderr," Can't find the CDP number between %d and %d in the file\n",firstcdp,endcdp);
      fprintf(stderr," The first CDP number is %ld in the file\n",tmcdp);
      trnum=(filelen-3600)/(240+bfh.SampleNumReel*4);

//      return trnum;
    }
    
//    fprintf(stderr,"bfh.SampleNumReel:%d  filelen=%u \n",bfh.SampleNumReel,filelen);

    data = Alloc2Float(trnum, bfh.SampleNumReel);
    move0 += trnum0*ml;
    /**** Read all data in same CDP ****/
    for( i=0;i<trnum;i++ ){
      if(FileSeek(fp,move0,0)==-1) break;
      FileRead(fp,data[i],4*bfh.SampleNumReel);
      for( j=0;j<bfh.SampleNumReel;j++ ){
        mm.ft = data[i][j];
        switch( bfh.DataFormat ){
          case 1:  /*IBM 4 Bytes Floating*/
                 data[i][j] = ibm_to_float(mm.ft);
                 break;
          case 2:  /* IBM 4 Bytes Fixed Point*/
                 data[i][j] = 0.0;
                 break;
          case 3:  /* IBM 2 Bytes Fixed Point*/
                 data[i][j] = 0.0;
                 break;
          case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
                 data[i][j] = 0.0;
                 break;
          case 5:  /* IEEE32 4 Bytes Floating*/
                 Swap4Char(mm.ch);
                 data[i][j] = (float)(mm.ft);
                 break;
          default:
                 data[i][j] = 0.0;
        }
      }
      move0 += ml;
    }
  }else{ /* trace length is vary */

    data = Alloc2Float(2, bfh.SampleNumReel);

  }

  *trdata = data;

  return trnum;
}
//---------------------------------------------------------------------------------

int get_segy_cdpNum(int fp, SEGY_BFILE_HEAD bfh, int firstcdp, int endcdp, float ***trdata)
{




}
//-----------------------------------------------------------------------------
int  get_segy_line_data2(FILE *fp, SEGY_BFILE_HEAD bfh,  float ***trdata,int BeginTrace,int EndTrace)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp          The pointer to the file readed
        bfh         The SEGY file binary file head
        BeginTrace  The begin number of trace
        EndTrace    The end number of trace
     Output:
        trdata      The SEGY data in same line. data[Traces][num]
     Return:
        Traces      The number of traces between BeginTrace and EndTrace.
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int  i,j,temp;
  int  num;
  long count,ml;
  float **data;
  VALUE_TYPE mm;


  __int64 Traces=EndTrace-BeginTrace;
  __int64 TrNum;

  if( bfh.DataFormat == 3 ) temp = 2;
  else if( bfh.DataFormat ==8 ) temp = 1;
  else temp = 4;

  ml = temp*bfh.SampleNumReel;

  if( *trdata!=NULL ) Free2Float(*trdata);

  TrNum = get_segy_trace_num(fp, bfh);
  if(TrNum<0)
   TrNum=(long)TrNum;

  if(Traces>TrNum)
   Traces=TrNum;

  count = 3840L;
  if( bfh.TraceLengthFlag==0 )
  {
    data = Alloc2Float(Traces,bfh.SampleNumReel);
    fseek(fp,count-128,0);
    fread(&mm.it,4,1,fp);  Swap4Char(mm.ch);  num = (int)mm.st[0];
    for( i=0;i<Traces;i++)
    {
      fseek(fp,count+(BeginTrace)*(temp*num+240),0);
      fread(data[i],temp,num,fp);
      for( j=0;j<num;j++ )
      {
        mm.ft = data[i][j];
        switch( bfh.DataFormat )
        {
          case 1:  /*IBM 4 Bytes Floating*/
              data[i][j] = ibm_to_float(mm.ft);
              break;
          case 2:   /*IBM 4 Bytes Fixed Point*/
              data[i][j] = 0.0;
              break;
          case 3:  /* IBM 2 Bytes Fixed Point*/
              data[i][j] = 0.0;
              break;
          case 4:  /* IBM 4 Bytes Fixed Point with gain code*/
              data[i][j] = 0.0;
              break;
          case 5:  /* IEEE32 4 Bytes Floating*/
              Swap4Char(mm.ch);
              data[i][j] = (float)(mm.ft);
              break;
          default:
              data[i][j] = 0.0;
        }
      }
      count += num*temp+240L;
    }
  }
  else
  {
    data = Alloc2Float(Traces,bfh.SampleNumReel);
    count=count+BeginTrace*(temp*bfh.SampleNumReel+240);
    for( i=0;i<Traces;i++)
    {
      fseek(fp,count,0);
      fread(data[i],temp,bfh.SampleNumReel,fp);
      for( j=0;j<bfh.SampleNumReel;j++ )
      {
        mm.ft = data[i][j];
        switch( bfh.DataFormat )
        {
          case 1:  /*IBM 4 Bytes Floating*/
              data[i][j] = ibm_to_float(mm.ft);
              break;
          case 2:  /* IBM 4 Bytes Fixed Point*/
              data[i][j] = 0.0;
              break;
          case 3: /*  IBM 2 Bytes Fixed Point*/
              data[i][j] = 0.0;
              break;
          case 4: /*  IBM 4 Bytes Fixed Point with gain code*/
              data[i][j] = 0.0;
              break;
          case 5: /*  IEEE32 4 Bytes Floating*/
              Swap4Char(mm.ch);
              data[i][j] = mm.ft;
              break;
          default:
              data[i][j] = 0.0;
        }
      }
      count += ml+240L;
    }
  }
  *trdata = data;
  return Traces;
}
//-----------------------------------------------------------------------------
int  get_su_line_data2(FILE *fp, segy *tr,  float ***trdata,int BeginTrace,int EndTrace)
/*-----------------------------------------------------------------------------------------------------------------------------------------
     Input:
        fp          The pointer to the file readed
        tr          The SU file trace head
        BeginTrace  The begin number of trace
        EndTrace    The end number of trace
     Output:
        trdata      The SU data in same line. data[Traces][num]
     Return:
        Traces      The number of traces between BeginTrace and EndTrace.
-----------------------------------------------------------------------------------------------------------------------------------------*/
{
  int  i,j;
  int  num;
  long count,ml;
  float **data;
  VALUE_TYPE mm;

  
  int TrNum;
  num=tr->ns;
  TrNum=get_file_length(fp)/(240+num*4);
  if(BeginTrace<tr->f2)
    BeginTrace=tr->f2;
  if(EndTrace>tr->f2+TrNum)
    EndTrace=tr->f2+TrNum;
  int Traces=EndTrace-BeginTrace;


 // Application->MessageBox("End="+IntToStr(EndTrace),"Begin="+IntToStr(BeginTrace));
  data = Alloc2Float(Traces,num);
  count=(BeginTrace-tr->f2)*(sizeof(float)*num+240) +240l;
  for( i=0;i<Traces;i++)
  {
     fseek(fp,count,0);
     fread(data[i],sizeof(float),num,fp);
     for( j=0;j<num;j++ )
     {
        mm.ft = data[i][j];
        Swap4Char(mm.ch);
        data[i][j] = mm.ft;
   /*     data[i][j] = ibm_to_float(mm.ft);*/
     }
     count += num*sizeof(float)+240L;
  }
  *trdata = data;
  return Traces;
}
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
int get_segy_cdp_data2(FILE *fp, SEGY_BFILE_HEAD bfh, int firstcdp, int endcdp, float ***trdata)
/*-----------------------------------------------------------------------------
Get all data in the same CDP from opened file.
        If NO<1, get the data in the first CDP.
-------------------------------------------------------------------------------------------------------------------------------------------
        Input:
             fp         The pointer to the file read
             bfh        The SEGY binary file head, lies 3200 to 3600 character
         firstcdp       The first cdp number in file
          endcdp        The last cdp number in file
        Output:
           trdata       The data between first cdp to end cdp
        Return:
           temsumtrace  The total number of traces between first cdp to end cdp

-------------------------------------------------------------------------------*/
{
    float **totaldata,**data;
    int total_cdp,temsumtrace;
    int *trace;

    if(firstcdp>endcdp)
       firstcdp=endcdp;
    if(firstcdp<=0)
       firstcdp=1;
    total_cdp=endcdp-firstcdp;
    trace=new int[total_cdp+1];
    temsumtrace=get_segy_cdp_total_trace(fp,bfh,firstcdp,endcdp);

    totaldata=Alloc2Float(temsumtrace, bfh.SampleNumReel);
    int tem=0;
    for(int i=firstcdp;i<=endcdp;i++)
    {
      trace[i-firstcdp]=get_segy_cdp_trace(fp, bfh,i);
      data = Alloc2Float(trace[i-firstcdp], bfh.SampleNumReel);
      trace[i-firstcdp]=get_segy_cdp_data1(fp,bfh,i,&data);
      for(int m=0;m<trace[i-firstcdp];m++)
          for(int n=0;n<bfh.SampleNumReel;n++)
             totaldata[tem+m][n]=data[m][n];
      tem=tem+trace[i-firstcdp];
      Free2Float(data);
    }
    *trdata = totaldata;
    delete []trace;
    return temsumtrace;
}
//-----------------------------------------------------------------------------
int get_segy_cdp_trace(FILE *fp, SEGY_BFILE_HEAD bfh,int NO)
/*-----------------------------------------------------------------------------
Get the number of trace in the same CDP from opened file.
-------------------------------------------------------------------------------------------------------------------------------------------
        Input:
             fp       The pointer to the file read
             bfh      The SEGY binary file head, lies 3200 to 3600 character
             NO       The CDP sequence number in file
        Return:
             trnum    the number of trace in NO. CDP

-------------------------------------------------------------------------------*/
{
  int   i,j,temp,trnum, trnum0, cdpnum, cdp;
  long  move,move0,tmcdp,ml, filelen;

  VALUE_TYPE mm;

 // if( NO<1 ) NO = 1;

  if( bfh.DataFormat==3 ) temp = 2;
  else if( bfh.DataFormat==8 ) temp = 1;
  else temp = 4;

  filelen = get_file_length(fp);

  ml = temp*bfh.SampleNumReel;
  move = 3600l+bfh.ExtTextNum*3200l;
  move0 = move+240;

  //if( *trdata!=NULL ) Free3Float(*trdata);

  trnum = 0;
  trnum0 = 0;
  cdpnum = 0;
  cdp = -10;
  if( bfh.TraceLengthFlag==1 )
  {
    /* Count the number of trace in same CDP */
    while( !feof(fp) )
    {
      fseek(fp, move+20, SEEK_SET);
      fread(&mm.lt,4,1,fp);         Swap4Char(mm.ch);  tmcdp = mm.it;
      if( tmcdp!=cdp )
      {
        cdp = tmcdp;
        cdpnum++;
        if( cdpnum==NO+1 ){ break; }
        else
        {
          trnum0 += trnum; trnum = 0;
        }
      }
      move += ml+240;
      trnum++;
      if( move>=filelen ) break;
    }
  }             
  return trnum;
}
//-----------------------------------------------------------------------------
int get_segy_cdp_total_trace(FILE *fp, SEGY_BFILE_HEAD bfh, int firstcdp, int endcdp)
/*-----------------------------------------------------------------------------
Get the total number of traces from firstcdp and endcdp  from opened file.
        If NO<1, get the data in the first CDP.
-------------------------------------------------------------------------------------------------------------------------------------------
        Input:
             fp         The pointer to the file read
             bfh        The SEGY binary file head, lies 3200 to 3600 character
         firstcdp       The first cdp number in file
          endcdp        The last cdp number in file

        Return:
           temsumtrace  The total number of traces between first cdp to end cdp

-------------------------------------------------------------------------------*/
{
    int total_cdp,temsumtrace;
    int *trace;

    if(firstcdp>endcdp)
       firstcdp=endcdp;
    if(firstcdp<=0)
       firstcdp=1;
    total_cdp=endcdp-firstcdp;
    trace=new int[total_cdp+1];
    temsumtrace=0;
    for(int i=firstcdp;i<=endcdp;i++)
    {
      trace[i-firstcdp]=get_segy_cdp_trace(fp,bfh,i);
      temsumtrace=temsumtrace+trace[i-firstcdp];
    }
    delete []trace;
    return temsumtrace;
}
//------------------------------------------------------------------------------
int cdp_number_trace(int *cdpno,int tracenum,int **cdp)
{
  int min,max;
  int *cdp_traces;
  int cdps;
  min=cdpno[0];
  max=cdpno[0];
  for(int i=1;i<tracenum;i++)
  {
    min=(min>cdpno[i])?cdpno[i]:min;
    max=(max<cdpno[i])?cdpno[i]:max;
  }
  cdps=max-min+1;
  cdp_traces=new int[cdps];
  for(int m=0;m<cdps;m++)
     cdp_traces[m]=0;

  for(int i=0;i<tracenum;i++)
    for(int j=min;j<=max;j++)
    {
       if(cdpno[i]==j)
          cdp_traces[j-min]++;
    }

  *cdp = cdp_traces;
  return cdps;
}
//-------------------------------------------------------------
void cdp_min_max(int *cdpno,int tracenum,int min,int max)
{
  min=cdpno[0];
  max=cdpno[0];
  for(int i=1;i<tracenum;i++)
  {
    min=(min>cdpno[i])?cdpno[i]:min;
    max=(max<cdpno[i])?cdpno[i]:min;
  }
}
