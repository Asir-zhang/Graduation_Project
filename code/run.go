package main

import (
	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
)

func main() {
	filename := "./segy/eage01_cdpm.sgy"
	setSource(filename)
	_, binaryHeader := getHeader()

	//判断是否采用IBM浮点格式
	isIbmFloat := binaryHeader.DataFormat == IBM_FLOAT
	//定义绘图工具
	p := plot.New()
	//每个道之间的间隔
	baseX := 0.0

	for i := int16(0); i < binaryHeader.NumDataTrace; i++ {
		// 读取道头，道数据
		traceHeader, data := nextTraceHeader(isIbmFloat)
		//采样间隔转换为秒
		dt := float64(traceHeader.SampleInterval / 1000)

		// 转换为float64类型
		floatData := make(plotter.XYs, len(data))
		max := 0.0
		min := 0.0
		for ii := range data {
			if float64(data[ii]) > max {
				max = float64(data[ii])
			} else if float64(data[ii]) < min {
				min = float64(data[ii])
			}
			floatData[ii].Y = float64(ii) * dt
			floatData[ii].X = float64(data[ii]) + baseX
		}
		baseX += max - min

		// 添加数据
		lpLine, err := plotter.NewLine(floatData)
		if err != nil {
			//panic("添加数据出错")
			panic(err)
		}
		//lpLine.Color = color.RGBA{R: 255, G: 0, B: 0, A: 255}
		p.Add(lpLine)
	}
	// 设置坐标轴和标题
	p.Y.Label.Text = "Time (s)"
	p.X.Label.Text = "Amplitude"
	p.Title.Text = "SEGY Trace"

	// 保存图像
	if err := p.Save(20*vg.Inch, 20*vg.Inch, "segy_trace.png"); err != nil {
		panic(err)
	}
}
