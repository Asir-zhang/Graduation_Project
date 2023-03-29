package main

import (
	"math"
)

func ibmToIeeeArr(ibmDataUint []uint32) []float32 {
	ieeeArr := make([]float32, len(ibmDataUint))
	for i, temp := range ibmDataUint {
		ieeeArr[i] = ibmToIeee(temp)
	}
	return ieeeArr
}

func ibmToIeee(ibmData uint32) float32 {
	// 获取IBM格式数据的符号位、指数和尾数
	sign := ibmData >> 31 & 0x01
	exponent := int(ibmData>>24&0x7f) - 64
	mantissa := float32(ibmData&0xffffff) / (1 << 24)
	// 计算IEEE格式数据的值
	ieeeData := math.Float32frombits(uint32(sign)<<31 | uint32(exponent+127)<<23 | uint32(mantissa*(1<<23)))
	return ieeeData
}
