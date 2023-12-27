# README.md

Create a pitch shifter object: PhaseVocoder(int fftSize, int bufferSize, float ampFactor, float pitchFactor, int numChannels);

* fftSize must be a power of 2。fftSize越大频率分辨度会更高，但延迟也会更高。我一般设成1024
* bufferSize determines the size of input and output buffer
* ampFactor scales the output audio, because overlap-add processing may result in louder output
* pitchFactor scales the frequency of output。0.5是低一八度，2.0是高一八度。不建议设在这两个数值以外
* numChannels是输入和输出的声道数，这里默认输入和输出的声道一样多

Use pitch shifting: applyPV(float *data, int numSamples, int channel);

* 将pitch shift用到一个单一的声道。如果音频是多声道则每个声道都要用
* data指向音频的输入和输出。因为是用JUCE框架开发的，输入和输出都会从data读和写。如果要分开请把输出代码`data[s] = ampScale*outBuf...`改成指向单独的输出指针
* numSamples是指电脑音频流的大小，和之前提到的bufferSize, fftSize等不是一个概念
