package main

import (
	"fmt"
	"github.com/sanandak/seg/segy"
	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
)

func main() {
	/*	// 打开 SEGY 文件
		f, err := os.Open("example.segy")
		if err != nil {
			panic(err)
		}
		defer f.Close()
	*/
	// 读取 SEGY 文件
	traces := segy.ReadSU("vel.segy")
	// 获取第一个道集的数据
	trace := traces[0]
	data := trace.Data

	// 绘制波形图
	p := plot.New()
	p.Title.Text = "SEGY Waveform"
	p.X.Label.Text = "Sample Index"
	p.Y.Label.Text = "Amplitude"
	points := make(plotter.XYs, len(data))
	for i := range data {
		points[i].X = float64(i)
		points[i].Y = float64(data[i])
	}
	line, err := plotter.NewLine(points)
	if err != nil {
		panic(err)
	}
	p.Add(line)
	if err := p.Save(4*vg.Inch, 4*vg.Inch, "segy_waveform.png"); err != nil {
		panic(err)
	}
	fmt.Println("SEGY waveform saved to segy_waveform.png")
}
