package main

import "math"

func ibmToFloat(ibm []int32) []float32 {
	ieee := make([]float32, len(ibm))

	for i, ibmInt := range ibm {
		// Extract sign, exponent, and fraction from IBM float
		sign := (uint32(ibmInt) & 0x80000000) != 0
		exp := int32(ibmInt >> 24 & 0x7f)
		frac := ibmInt & 0xffffff

		// Calculate the exponent and mantissa of the IEEE float
		var ieeeExp int32
		var ieeeMant uint32
		if exp == 0 {
			// Zero or denormalized value
			ieeeExp = -126
			for frac != 0 {
				ieeeExp--
				frac <<= 1
			}
			ieeeMant = uint32(frac << 8)
		} else {
			// Normalized value
			ieeeExp = exp - 64
			ieeeMant = uint32(frac << 7)
			ieeeMant |= uint32(1) << 31
		}

		// Combine sign, exponent, and mantissa to get IEEE float
		ieeeInt := uint32(ieeeExp+127) << 23
		ieeeInt |= ieeeMant
		if sign {
			ieeeInt |= 0x80000000
		}
		ieee[i] = math.Float32frombits(ieeeInt)
	}

	return ieee
}
