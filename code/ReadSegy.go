package main

import (
	"encoding/binary"
	"os"
)

type TextHeader [3200]byte

const (
	IBM_FLOAT = 1 // IBM浮点格式
)

var file *os.File

type BinaryHeader struct {
	NoJob                 int32
	NoLine                int32
	NoReel                int32
	NumDataTrace          int16
	NumAuxiTrace          int16
	SampleRatioReel       int16
	SampleRatioField      int16
	SampleNumReel         int16
	SampleNumField        int16
	DataFormat            int16
	CDPFold               int16
	TraceSortCode         int16
	VerticalSumCode       int16
	SweepFreqStart        int16
	SweepFreqEnd          int16
	SweepLength           int16
	SweepTypeCode         int16
	SweepTraceNum         int16
	SweepTaperLenStart    int16
	SweepTaperLenEnd      int16
	SweepTaperType        int16
	CorrelDataTrace       int16
	BinaryGainRecover     int16
	AmpRecoverMethod      int16
	MeasurementSys        int16
	ImpulseSignal         int16
	VibratoryPolarityCode int16
	Reserved1             [60]int32
	Revision              uint16
	TraceLengthFlag       int16
	ExtTextNum            int16
	Reserved2             [94]int8
}

type TraceHeader struct {
	TraceNoLine           int32
	TraceNoReel           int32
	RecordNoField         int32
	TraceNoField          int32
	SourceNo              int32
	CDPNo                 int32
	TraceNoCDP            int32
	TraceCode             int16
	TraceNumVerSum        int16
	TraceNumHorSum        int16
	DataType              int16
	DistanceStoR          int32
	ElevReceiver          float32
	ElevSource            float32
	DepthSource           float32
	ElevReceiverDatum     float32
	ElevSourceDatum       float32
	DepthSourceWater      float32
	DepthReceiverWater    float32
	ScalerE               int16
	ScalerC               int16
	SourceCoordinateX     float32
	SourceCoordinateY     float32
	ReceiverCoordinateX   float32
	ReceiverCoordinateY   float32
	CoordinateUnit        int16
	WeatherVelocity       int16
	SubweatherVelocity    int16
	UpholeTimeSource      int16
	UpholeTimeReceiver    int16
	StaticCorrectSource   int16
	StaticCorrectReceiver int16
	TotalStatic           int16
	LagTimeA              int16
	LagTimeB              int16
	DelayTime             int16
	MuteTimeStart         int16
	MuteTimeEnd           int16
	SampleNum             int16
	SampleInterval        int16
	GainType              int16
	GainConstant          int16
	InitalGain            int16
	Correlated            int16
	SweepFreqStart        int16
	SweepFreqEnd          int16
	SweepLength           int16
	SweepType             int16
	SweepTaperLStart      int16
	SweepTaperLEnd        int16
	SweepTaperType        int16
	AliasFreq             int16
	AliasSlope            int16
	NotchFreq             int16
	NotchSlope            int16
	LowCutFreq            int16
	HighCutFreq           int16
	LowCutSlope           int16
	HighCutSlope          int16
	Year                  int16
	Day                   int16
	Hour                  int16
	Minute                int16
	Second                int16
	TimeBasisCode         int16
	TraceWeightFactor     int16
	GeophoneNoRoll        int16
	GeophoneNoFirstTrace  int16
	GeophoneNoLastTrace   int16
	GapSize               int16
	Overtravel            int16
	CDPPosX               float32
	CDPPosY               float32
	InLineNo              int32
	CrossLineNo           int32
	ShotPointNum          float32
	ScaleS                int16
	ValueUnit             int16
	TransConstantB        int32
	TransConstantE        int16
	TransUnit             int16
	DeviceID              int16
	ScaleT                int16
	Reserved              [6]int32
}

// 绑定源文件
func setSource(filename string) {
	tfile, err := os.Open(filename)
	if err != nil {
		panic("打开文件出错")
		panic(err)
	}
	file = tfile
}

// 获取文件头
func getHeader() (TextHeader, BinaryHeader) {
	if file == nil {
		panic("未绑定源文件")
	}
	var textHeader TextHeader
	if err := binary.Read(file, binary.BigEndian, &textHeader); err != nil {
		panic("读取文本文件头出错")
		panic(err)
	}
	var binaryHeader BinaryHeader
	if err := binary.Read(file, binary.BigEndian, &binaryHeader); err != nil {
		panic("读取二进制文件头出错")
		panic(err)
	}
	return textHeader, binaryHeader

}

// 获取道头,道数据
func nextTraceHeader(isIbmFloat bool) (TraceHeader, []float32) {
	if file == nil {
		panic("未绑定源文件")
	}
	var traceHeader TraceHeader
	if err := binary.Read(file, binary.BigEndian, &traceHeader); err != nil {
		panic("读取道头出错")
		panic(err)
	}
	// 解析地震数据
	var data []float32
	if isIbmFloat {
		// IBM浮点格式，需要进行特殊处理
		ibmData := make([]uint32, traceHeader.SampleNum)
		if err := binary.Read(file, binary.BigEndian, &ibmData); err != nil {
			panic(err)
		}
		data = ibmToIeeeArr(ibmData)
	} else {
		// IEEE浮点格式，直接读取即可
		ieeeData := make([]float32, traceHeader.SampleNum)
		if err := binary.Read(file, binary.BigEndian, &ieeeData); err != nil {
			panic(err)
		}
		data = ieeeData
	}
	return traceHeader, data
}
