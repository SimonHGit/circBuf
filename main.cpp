#include <iostream>
#include <memory>

template <class T, int t_size, int _offset,int _blockSize>
class circular_buffer {
public:
    explicit circular_buffer(int delay = 0) :
            buf( std::unique_ptr<T[]>(new T[t_size])),
            max_size_(t_size){
        offset = _offset;
        blockSize = _blockSize;
        if (delay) writePos = blockSize-offset;
    }

    size_t size() const
    {
        size_t size = max_size_;
        return size;
    }
    void clearRegion(int startSample, int numSamples){
        for (int i = 0; i < numSamples; i++) {
            buf[(startSample+i)%max_size_] = 0;
        }
    }
    void clearNextRegion(){
        clearRegion(writePos+offset,blockSize);
    }
    void show(){
        for (int i = 0; i < max_size_; i++) {
            std::cout << buf[i] << " ";
        }
        std::cout << std::endl;
    }
    int incWritePos(int stepSize){
        int tmp = writePos;
        writePos += stepSize;
        if (writePos>=max_size_) writePos = writePos % max_size_;
        return tmp;
    }
    void writeNextBlock(float *input, int numSamples ){
        for (int i = 0; i < numSamples; i++) {
            buf[incWritePos(1)] = input[i];
        }
    }

    void addSamples(float *input, int numSamples , int incWritePointerBy = 0){
        int tmp = writePos;
        clearNextRegion();
        for (int i = 0; i < numSamples; i++) {
            buf[incWritePos(1)] += input[i];
        }
        if (incWritePointerBy) {
            writePos = tmp;
            incWritePos(incWritePointerBy);
        }
    }

    bool nextReadIterPossible(){
        if (writePos == max_size_ / 2) {
            return ((readPos + blockSize )%max_size_<= max_size_ / 2);
        }
        else if (writePos == 0) {
            return (readPos + blockSize <= max_size_);
        }
    }

    int incReadPos(int stepSize)
    {
        int tmp = readPos;
        readPos += stepSize;
        if (readPos>=max_size_) readPos=readPos % max_size_;
        return tmp;
    }
    void exportSection(int numSamples, float *target, bool incReadPosByOffset = false){
        int tmp = readPos;
        for(int i=0; i<numSamples; i++)
            target[i] = buf[incReadPos(1)];
        if (incReadPosByOffset) {
            readPos = tmp;
            incReadPos(offset);
        }

    }

private:
    std::unique_ptr<T[]> buf;
    const size_t max_size_;
    int readPos=0;
    int writePos = 0;
    int offset;
    int blockSize;

};

int main(){

    const int nInputSamples = 32;
    const int fftSize = 4;
    const int offset = 2;

    circular_buffer<float,2*nInputSamples,offset,fftSize> inBuffer;
    circular_buffer<float,2*nInputSamples,offset,fftSize> outBuffer(fftSize);
//    std::cout << "empty circular buffers: " << std::endl;
//    inBuffer.show();
//    outBuffer.show();

    float output[nInputSamples];
    float input[nInputSamples];

    float window[fftSize] ={ 0, 1, 1, 0};
    int jjj = 2;

    //fill inputBuffer
    for (int k = 0; k < 6; k++) {
        for (int i = 0; i < nInputSamples; i++) {
            input[i] = (float)jjj++;
        }
//        std::cout << std::endl << "-----------------new iteration: " << k << std::endl;

        //write new input block
        inBuffer.writeNextBlock(input,nInputSamples);
//        std::cout << std::endl << "input buffer: " << std::endl;
//        inBuffer.show();

        while(inBuffer.nextReadIterPossible()){
            //copy values into workingArray
            float workingArray[4];
            inBuffer.exportSection(4,workingArray,true);
            for (int i = 0; i < 4; i++) {
                workingArray[i] *= window[i] ;
                //workingArray[i]*=workingArray[i];
            }
            //write data to outputBuffer
            outBuffer.addSamples(workingArray,fftSize,offset);
        }

//        std::cout << "output buffer post iteration " << ": " << std::endl;
//        outBuffer.show();

        //write data to outputstream:
        outBuffer.exportSection(nInputSamples,output);
//        std::cout << "output stream" << std::endl;
        for (int i = 0; i < nInputSamples; i++) std::cout << output[i] << " ";
        std::cout << std::endl;
    }

    return 0;
}