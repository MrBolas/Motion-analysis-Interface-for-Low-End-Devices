//
//  main.cpp
//  OpenCv
//
//  Created by João Bolas on 03/03/14.
//  Copyright (c) 2014 João Bolas. All rights reserved.
//

#include "opencv/cv.h"
#include "opencv/cvaux.h"
#include "opencv/cxmisc.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include <curses.h>
#include <unistd.h>
#include <math.h>


using namespace cv;
using namespace std;

#pragma mark - helper functions

void frameSubtraction(cv::Mat& intruder, cv::Mat& background, cv::Mat& finalFrame){
    //Nesta função vamos remover a frame original o background atraves da remoção da diferença absoluta
    blur(intruder, intruder, cv::Size(4,4));
    blur(background, background, cv::Size(4,4));
    finalFrame= abs(intruder-background);//Frame final fica na variavel finalFrame
}

int selectingRegionOfInterest(char* id,int xAxis, int yAxis, int roiWidth, int roiHeight,Mat& frame, Mat& roiRect){
    
    float frameThreshold = 0.9;
    float numberOfPixels = roiHeight*roiWidth;
    float whitePixels=countNonZero(roiRect);
    float percentOfWhitePixels= (whitePixels)/numberOfPixels;
    
    roiRect=frame(Rect(xAxis,yAxis,roiWidth,roiHeight));
    id= strcat(id, " ROI");
    
    if (numberOfPixels*frameThreshold<countNonZero(roiRect)) {
        return 1;
    }
    return 0;
}

void DrawingRegionsOfInterest(Mat& mainFrame,int xAxis, int yAxis, int roiWidth, int roiHeight){
    rectangle(mainFrame, cvPoint(xAxis,yAxis),cvPoint(xAxis+roiWidth, yAxis+roiHeight), cv::Scalar(100, 100, 200), 2, CV_AA);
}

int selectingRegionOfInterestWithDrawing(char* id,int xAxis, int yAxis, int roiWidth, int roiHeight,Mat& frame, Mat& roiRect, Mat& mainColorFrame){
    
    float frameThreshold = 0.9;
    float numberOfPixels = roiHeight*roiWidth;
    //float whitePixels=countNonZero(roiRect);
    //float percentOfWhitePixels= (whitePixels)/numberOfPixels;
    
    roiRect=frame(Rect(xAxis,yAxis,roiWidth,roiHeight));
    id= strcat(id, " ROI");
    
    DrawingRegionsOfInterest(mainColorFrame, xAxis, yAxis, roiWidth, roiHeight);
    
    if (frameThreshold<countNonZero(roiRect)/numberOfPixels) {
        return 1;
    }
    return 0;
}

void blurMe (Mat& blurFrame){
    blur(blurFrame, blurFrame, cv::Size(4,4));
}

int numberOfNonZero(Mat& frame,int xAxis, int yAxis, int roiWidth, int roiHeight){
    int count=countNonZero(frame(Rect(xAxis,yAxis,roiWidth,roiHeight)));
    return count;
}

#pragma mark - ROI position functions

int xPositionOfROI(float obstructionIndex,int coordX,int coordY, int widthOfFrame,int numberOfCollumns,bool isItLeft,int yOffsetPosition){
    /* yOffsetPosition server para criar um offset logico dinamico dependendo de qual a altura do ROI apresentando um display como o seguinte
     
        OO      OO
        OO      OO
     OO            OO
     OO            OO
        OO      OO
        OO      OO
     
     */
    
    int collumnWidth=widthOfFrame/numberOfCollumns;
    
    if (isItLeft) {
        if ((coordX*(obstructionIndex))*collumnWidth<1) {
            return 1;
        }else{
            switch (yOffsetPosition) {
                case 1:
                    return ((coordX*(obstructionIndex)*0.5)*collumnWidth);
                    break;
                case 2:
                    return ((coordX*(obstructionIndex)*0.3)*collumnWidth);
                    break;
                case 3:
                    return ((coordX*(obstructionIndex)*0.5)*collumnWidth);
                    break;
                default:
                    return ((coordX*(obstructionIndex))*collumnWidth);
                    break;
            }
        }
    }
    else{
        if ((((numberOfCollumns-coordX)*(obstructionIndex))+coordX)*collumnWidth>widthOfFrame) {
            //printf("case 0 %d ", widthOfFrame-20);
            return widthOfFrame-20;
        }else{
            switch (yOffsetPosition) {
                case 1:
                    // printf(" case 1 %f ", ((((numberOfCollumns-coordX-1)*obstructionIndex)*0.5+coordX))*collumnWidth);
                    return ((((numberOfCollumns-coordX-1)*obstructionIndex)*0.5+coordX))*collumnWidth;
                    break;
                case 2:
                    // printf("case 2 %f ",((((numberOfCollumns-coordX-1)*obstructionIndex)*0.6+coordX))*collumnWidth);
                    return ((((numberOfCollumns-coordX-1)*obstructionIndex)*0.7+coordX))*collumnWidth;
                    break;
                case 3:
                    return ((((numberOfCollumns-coordX-1)*obstructionIndex)*0.5+coordX))*collumnWidth;
                    break;
                default:
                    return (((numberOfCollumns-coordX)*obstructionIndex)+coordX)*collumnWidth;
                    break;
            }
        }
    }
}

int widthOfRoi(double obstructionIndex,int coordX,int coordY,int widthOfFrame,int numberOfCollumns,bool isItLeft,int yOffsetPosition){
    double defalutSize=50;
    
    
    int xPosition=xPositionOfROI(obstructionIndex, coordX, coordY, widthOfFrame, numberOfCollumns, isItLeft, yOffsetPosition);
    
    //printf(" xposition: %d ",xPosition);
    
    if (xPosition>=widthOfFrame-30&&!isItLeft) {
        //  printf("29 1");
        return 29;
    }
    
    if (!isItLeft&&(int)(defalutSize/obstructionIndex)>(widthOfFrame-xPosition)) {
        //   printf("%d 2",(widthOfFrame-xPosition));
        return ((widthOfFrame-xPosition));
    }
    else{
        //   printf(" defalutSize/obs+coordX*collumWidth %d ",(int)(defalutSize/obstructionIndex+coordX*collumWidth));
        //   printf("%d 3",(int)(defalutSize/obstructionIndex));
        return (int)((defalutSize/obstructionIndex));
    }
}

int yPositionOfROI(float obstructionIndex,int coordX,int coordY,int heightOfFrame,int numberOfRows,int yOffsetPosition){
    
    int rowHeight=heightOfFrame/numberOfRows;
    
    switch (yOffsetPosition) {
        case 1:
            if ((((coordY)*obstructionIndex)*0.4)*rowHeight<0) {
                return 10;
            }else
                return ((coordY*obstructionIndex)*0.4)*rowHeight;
            
            break;
        case 2:
            return ((coordY*obstructionIndex))*rowHeight;
            
            break;
        case 3:
            if (((coordY*obstructionIndex)*1.4)*rowHeight>heightOfFrame-10) {
                return heightOfFrame-10;
            }
            //return 400;
            return ((coordY*obstructionIndex)*1.4)*rowHeight;
            break;
        default:
            return (coordY);
            break;
    }
    
    
    return 0;
}

int heightOfRoi(float obstructionIndex,int coordX,int coordY,int heightOfFrame,int numberOfRows,int yOffsetPosition){
    int defaultSize=50;
    
    int yPosition=yPositionOfROI(obstructionIndex, coordX, coordY, heightOfFrame, numberOfRows, yOffsetPosition);
    
    //printf(" yposition: %d ",yPosition);
    
    if (yPosition>=heightOfFrame-10) {
        //printf(" %d 1",heightOfFrame-10);
        return 10;
    }
    
    if ((defaultSize/obstructionIndex)>(heightOfFrame-yPosition)) {
        //printf(" %d 2",(heightOfFrame-yPosition));
        return heightOfFrame-yPosition;
    }
    else{
        //printf(" %d 3",(int)(defaultSize/obstructionIndex));
        return (int)defaultSize/obstructionIndex;
    }
    
    return 0;
}



#pragma mark - human position

float humanPositionRatio(Mat& presentFrame,int frameWidth,int frameHeight){
    
    float numberOfRatioPixels=countNonZero(presentFrame);
    float obstructionIndex=numberOfRatioPixels/(frameHeight*frameWidth);

    return obstructionIndex;
}

int humanPositionX(Mat& presentFrame,int coordX,int frameWidth,int frameHeight,int numberOfColumns){
    
    int roiWidth = frameWidth/numberOfColumns;
    int tempScale=1;
    int threshold=numberOfColumns,window=numberOfColumns/4;
    double sensible=0.8,numberOfPixelsRoi=roiWidth*frameHeight;
    float result;
    int xPositions[numberOfColumns],counter=0,failSafeCoordX=coordX;
    Mat roi;
    //detection of non white pixels on the roi
    
    if (coordX<numberOfColumns) {
        threshold=coordX;
    }
    
    if (threshold-window/2<0||threshold+window/2>numberOfColumns) {
        for (int i=0; i<numberOfColumns; i++) {
            
            if (tempScale<countNonZero(presentFrame(Rect(i*roiWidth,1,roiWidth,frameHeight-1)))&&sensible<(countNonZero(presentFrame(Rect(i*roiWidth,1,roiWidth,frameHeight-1)))/numberOfPixelsRoi)){
                tempScale=countNonZero(presentFrame(Rect(i*roiWidth,1,roiWidth,frameHeight-1)));
                xPositions[counter]=i;
                counter++;
            }
            
        }
        
        if (counter!=0) {
            coordX=0;
            for (int i=0; i<counter; i++) {
                coordX+=xPositions[i];
            }
            
            result=((float)coordX)/counter;
            return (int)result;
        }
    }
    else if(threshold-window/2>0&&threshold+window/2<numberOfColumns){
        for (int i=threshold-window/2; i<threshold+window/2; i++) {
            
            if (tempScale<countNonZero(presentFrame(Rect(i*roiWidth,1,roiWidth,frameHeight-1)))&&sensible<(countNonZero(presentFrame(Rect(i*roiWidth,1,roiWidth,frameHeight-1)))/numberOfPixelsRoi)){
                tempScale=countNonZero(presentFrame(Rect(i*roiWidth,1,roiWidth,frameHeight-1)));
                //                coordX=i;
                xPositions[counter]=i;
                counter++;
            }
        }
        
        if (counter!=0) {
            coordX=0;
            for (int i=0; i<counter; i++) {
                coordX+=xPositions[i];
            }
            
            
            result=((float)coordX)/counter;
            //printf("%d %d %f\n",coordX,counter,result);
            return (int)result;
        }
    }
    //    else{
    //        coordX=numberOfColumns/2;
    //    }
    
    //printf("%d\n",coordX);
    return coordX;
}

int humanPositionY(Mat& presentFrame,int coordY,int frameWidth,int frameHeight,int numberOfRows){
    
    double roiHeight =frameHeight/numberOfRows,coordYAverage=0,sensitivity=0.7;
    double numberOfPixelsRoi=frameWidth*roiHeight;
    int yPositions[numberOfRows],maxYPositions=0,k=0,counter = 0;
    bool validYPositions[numberOfRows];
    float temp;
    
    //detection of non white pixels on the roi
    for (int i=1; i<numberOfRows-1; i++) {
        
        if (sensitivity<((countNonZero(presentFrame(Rect(1,i*roiHeight,frameWidth-1,roiHeight))))/(numberOfPixelsRoi))){
            yPositions[i]=countNonZero(presentFrame(Rect(1,i*roiHeight,frameWidth-1,roiHeight)));
        }
    }
    
    temp=0;
    int tempIndex=0;
    
    
    for (counter=0; counter<((numberOfRows)); counter++) {
        if (sensitivity<((countNonZero(presentFrame(Rect(1,counter*roiHeight,frameWidth-1,roiHeight))))/(numberOfPixelsRoi))&&yPositions[counter]!=0) {
            yPositions[k]=counter;
            k++;
        }
    }
    
    
    for (int i =0; i<k; i++) {
        coordYAverage+=yPositions[i];
    }
    
    coordYAverage=coordYAverage/k;
    
    for (int i=0; i<numberOfRows; i++) {
        // printf("%d ",yPositions[i]);
    }
    // printf("\n");
    
    //return tempIndex;
    
    //coordY=coordYAverage/counter;
    
    
    return coordYAverage;
}


#pragma mark - system control functions

int changeVolume(int volume){
    
    stringstream ss;
    ss << "osascript -e ' set volume output volume " << volume << " ' \n";
    system(ss.str().c_str());
    
    if (volume<0) {
        return 0;
    }
    else if (volume>100){
        return 100;
    }
    
    return volume;
}

void muteSystem(bool muted){
    
    stringstream ss;
    ss << "osascript -e ' set volume output muted " << muted << " ' \n";
    system(ss.str().c_str());
    
    if (muted) {
        printf("muted? Yes ");
    }else if (!muted){
        printf("muted? No ");
    }
    
}

void systemKeyPressed(int direction){
    
    stringstream ss;
    
    switch (direction) {
        case 126 :
            
            ss << "osascript -e ' key down (key code 126) ' \n";
            system(ss.str().c_str());
            
            break;
            
        case 123 :
            
            ss << "osascript -e ' key down (key code 123) ' \n";
            system(ss.str().c_str());
            
            break;
            
        case 125 :
            
            ss << "tell application \"System Events\" to key code 31 using control down\n";
            system(ss.str().c_str());
            
            break;
            
        case 124 :
            
            ss << "osascript -e ' key down (key code 124) ' \n";
            system(ss.str().c_str());
            
            break;
            
        default:
            printf(" %i is not a valid directional Key \n", direction);
            break;
    }
    
}

#pragma mark - main function

int main() {
    VideoCapture stream0(0);   //0 is the id of video device.0 if you have only one camera.
    printf("%f\n",stream0.get(CV_CAP_PROP_FPS));
    stream0.set(CV_CAP_PROP_FPS, 2.0);
    printf("%f\n",stream0.get(CV_CAP_PROP_FPS));
    
    if (!stream0.isOpened()) { //check if video device has been initialised
        cout << "cannot open camera";
    }
    
    // Declaração das Frames
    Mat cameraFrame; // frame da feed
    Mat clearFrame; //frame do background
    Mat backgroundRemoved; // frame da feed - background
    
    Mat oddCameraFrame,evenCameraFrame,colorOddCameraFrame,colorEvenCameraFrame,roiRect,movementDetectionFrame; //frames que detectam movimento
    bool newFrameIsOdd = true,isSetupDone = false,multiplexing=false;
    int multiplexingCounter=0;
    
    //controling System Variables
    int volume= 50,volumeSensibility=10,up=126,down=125,right=124,left=123,confirm=36;
    bool muted= false;
    bool multiplexingTask=false;
    
    //Finding the Human variabels
    int numberOfColumns=64;   //Limit is frame Width altough that is very CPU intensive
    int numberOfRows=32;      //Limit is frame Height altough that is very CPU intensive
    int coordX = numberOfColumns/2,newCoordX;
    int coordY=numberOfRows/2;
    int coordRatio=0;
    int frameWidth=640,frameHeight=480;
    int failSafeCounter=0;
    float overSamplingX=0;
    float obstructionIndex=0;
    
    //Start button :p
    //    getchar();
    stream0.read(clearFrame);
    
    
    // One Infinite Loop
    while (1) {
        stream0 >> cameraFrame;   // get a new frame from camera
        
        if (colorOddCameraFrame.empty()) {
            colorOddCameraFrame=cameraFrame;
            
        }
        else if (colorEvenCameraFrame.empty()) {
            colorEvenCameraFrame=cameraFrame;
            cv::cvtColor(colorEvenCameraFrame, evenCameraFrame, CV_BGR2GRAY);
        }
        else {
            isSetupDone=true;
            if (newFrameIsOdd) {
                cv::cvtColor(cameraFrame, oddCameraFrame, CV_BGR2GRAY);
                blurMe(oddCameraFrame);
                blurMe(evenCameraFrame);
                frameSubtraction(oddCameraFrame, evenCameraFrame, movementDetectionFrame);
                newFrameIsOdd=false;
                //                blurMe(movementDetectionFrame);
                //imshow("Odd Frame", oddCameraFrame);
                imshow("Movement Detection", movementDetectionFrame);
                //printf("%i \n",countNonZero(movementDetectionFrame));
            }
            else if (!newFrameIsOdd){
                cv::cvtColor(cameraFrame, evenCameraFrame, CV_BGR2GRAY);
                blurMe(oddCameraFrame);
                blurMe(evenCameraFrame);
                frameSubtraction(oddCameraFrame, evenCameraFrame, movementDetectionFrame);
                newFrameIsOdd=true;
                //                blurMe(movementDetectionFrame);
                //imshow("Even Frame", evenCameraFrame);
                imshow("Movement Detection", movementDetectionFrame);
                //printf("%i \n",countNonZero(movementDetectionFrame));
            }
        }
        
        if(waitKey(150)!=0){ }
        
        
        
        //Tratamento de Imagem (Isolamento do Intruso)
        Mat removedBackGround;
        Mat removedBackGroundGrey;
        
        frameSubtraction(cameraFrame, clearFrame, removedBackGround);     //Função de remoção de background por anulamento de frames
        
        cv::cvtColor(removedBackGround, removedBackGroundGrey, CV_BGR2GRAY);            //conversao de RGB para GreyScale
        
        
#pragma mark - Region of interest Variables
        
        Mat ROIul,ROIml,ROIll,ROIur,ROImr,ROIlr;
        char upLeft[16] = "upper left",lowerRight[16] = "lower right",midRight[16] = "mid right",upRight[16] = "upper right",lowerLeft[16] = "lower left",midLeft[16] = "mid left";
        int WidthOfROI=200,HeightOfROI=160;
        
        Point UpperLeftCornerOfUpperLeftROI(1,1),LowerRightCornerOfUpperLeftROI(WidthOfROI,HeightOfROI);
        Point UpperLeftCornerOfMidLeftROI(1,HeightOfROI),LowerRightCornerOfMidLeftROI(WidthOfROI,2*HeightOfROI);
        Point UpperLeftCornerOfLowerLeftROI(1,2*HeightOfROI),LowerRightCornerOfLowerLeftROI(WidthOfROI,3*HeightOfROI);
        
        Point UpperLeftCornerOfUpperRightROI(frameWidth-WidthOfROI,1),LowerRightCornerOfUpperRightROI(frameWidth,HeightOfROI);
        Point UpperLeftCornerOfMidRightROI(frameWidth-WidthOfROI,HeightOfROI),LowerRightCornerOfMidRightROI(frameWidth,2*HeightOfROI);
        Point UpperLeftCornerOfLowerRightROI(frameWidth-WidthOfROI,2*HeightOfROI),LowerRightCornerOfLowerRightROI(frameWidth,3*HeightOfROI);
        
#pragma mark - tracking
        if (isSetupDone) {
            
            obstructionIndex=humanPositionRatio(movementDetectionFrame, frameWidth, frameHeight);
            
            if (obstructionIndex<0.9&&obstructionIndex>0.4) {
                
                newCoordX=humanPositionX(movementDetectionFrame,coordX,frameWidth,frameHeight,numberOfColumns);
                
                if(coordX==newCoordX)failSafeCounter++;
                
                if (failSafeCounter>60) {
                    failSafeCounter=0;
                    newCoordX=humanPositionX(movementDetectionFrame,numberOfColumns/2,frameWidth,frameHeight,numberOfColumns);
                }
                
                coordX=newCoordX;
                
                coordY=humanPositionY(movementDetectionFrame,0, frameWidth, frameHeight,numberOfRows);
                
                DrawingRegionsOfInterest(cameraFrame, coordX*(frameWidth/numberOfColumns), 1, (frameWidth/numberOfColumns), frameHeight-1);
                DrawingRegionsOfInterest(cameraFrame, 1, coordY*(frameHeight/numberOfRows), frameWidth-1, (frameHeight/numberOfRows));
                
                    }
        }
        
#pragma mark - Application of Region of interest
        if (isSetupDone&&!multiplexing) {
            
            if (obstructionIndex<0.9&&obstructionIndex>0.4) {
                
                if (selectingRegionOfInterestWithDrawing(upLeft,xPositionOfROI(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true,1), yPositionOfROI(obstructionIndex, coordX, coordY, frameHeight, numberOfRows,1), widthOfRoi(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true, 1), heightOfRoi(obstructionIndex, coordX, coordY, frameHeight, numberOfRows, 1), movementDetectionFrame, ROIul,cameraFrame)) {systemKeyPressed(up);}
                //imshow(upLeft, ROIul);
                
                //            selectingRegionOfInterest(midLeft,1, 160, 200, 160, movementDetectionFrame, ROIml);
                if (selectingRegionOfInterestWithDrawing(midLeft,xPositionOfROI(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true,2), yPositionOfROI(obstructionIndex, coordX, coordY, frameHeight, numberOfRows,2),widthOfRoi(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true, 2), heightOfRoi(obstructionIndex, coordX, coordY, frameHeight, numberOfRows, 2), movementDetectionFrame, ROIml,cameraFrame)) {systemKeyPressed(left);}
                //imshow(midLeft, ROIml);
                
                //            selectingRegionOfInterest(lowerLeft,1, 320, 200, 160, movementDetectionFrame, ROIll);
                if (selectingRegionOfInterestWithDrawing(lowerLeft, xPositionOfROI(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true,3), yPositionOfROI(obstructionIndex, coordX, coordY, frameHeight, numberOfRows,3),widthOfRoi(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true, 3), heightOfRoi(obstructionIndex, coordX, coordY, frameHeight, numberOfRows, 3), movementDetectionFrame, ROIll,cameraFrame)) {systemKeyPressed(down);}
                //imshow(lowerLeft, ROIll);
                
                //          selectingRegionOfInterest(upRight,440, 1, 200, 160, movementDetectionFrame, ROIur);
                if (selectingRegionOfInterestWithDrawing(upRight,xPositionOfROI(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, false, 1), yPositionOfROI(obstructionIndex, coordX, coordY, frameHeight, numberOfRows,1), widthOfRoi(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, false, 1),  heightOfRoi(obstructionIndex, coordX, coordY, frameHeight, numberOfRows, 1), movementDetectionFrame, ROIur,cameraFrame)) {systemKeyPressed(confirm);}
                //imshow(upRight, ROIur);
                
                //            selectingRegionOfInterest(midRight,440, 160, 200, 160, movementDetectionFrame, ROImr);
                if (selectingRegionOfInterestWithDrawing(midRight,xPositionOfROI(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, false, 2), yPositionOfROI(obstructionIndex, coordX, coordY, frameHeight, numberOfRows,2),widthOfRoi(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, false, 2),  heightOfRoi(obstructionIndex, coordX, coordY, frameHeight, numberOfRows, 2), movementDetectionFrame, ROImr,cameraFrame)) {systemKeyPressed(right);}
                //imshow(midRight, ROImr);
                
                if (selectingRegionOfInterestWithDrawing(lowerRight,xPositionOfROI(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, false, 3), yPositionOfROI(obstructionIndex, coordX, coordY, frameHeight, numberOfRows,3), widthOfRoi(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, false, 3),  heightOfRoi(obstructionIndex, coordX, coordY, frameHeight, numberOfRows, 3), movementDetectionFrame, ROIlr,cameraFrame)) {multiplexing=true;}
                
            }
        }
        
        
#pragma mark - Combined Region of Interest
        
        if (isSetupDone&&multiplexing) {
            multiplexingCounter++;
            
            
            //lower right and lower left
            if (selectingRegionOfInterestWithDrawing(lowerLeft, xPositionOfROI(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true,3), yPositionOfROI(obstructionIndex, coordX, coordY, frameHeight, numberOfRows,3),widthOfRoi(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true, 3), heightOfRoi(obstructionIndex, coordX, coordY, frameHeight, numberOfRows, 3), movementDetectionFrame, ROIll,cameraFrame)&&multiplexingTask==false) {
                multiplexingTask=true;
                printf("mute system");
                
                if (muted) {
                    muteSystem(false);
                    muted=false;
                    putText(cameraFrame, " UNMUTED  ", cvPoint(frameWidth-WidthOfROI,frameHeight-HeightOfROI/2),FONT_HERSHEY_COMPLEX_SMALL, 1.0, cvScalar(200,200,250), 1, CV_AA);
                }
                else{
                    muteSystem(true);
                    muted=true;
                    putText(cameraFrame, " MUTED  ", cvPoint(frameWidth-WidthOfROI,frameHeight-HeightOfROI/2),FONT_HERSHEY_COMPLEX_SMALL, 1.0, cvScalar(200,200,250), 1, CV_AA);
                }
            }
            
            //lower right and mid left
            if (selectingRegionOfInterestWithDrawing(midLeft,xPositionOfROI(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true,2), yPositionOfROI(obstructionIndex, coordX, coordY, frameHeight, numberOfRows,2),widthOfRoi(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true, 2), heightOfRoi(obstructionIndex, coordX, coordY, frameHeight, numberOfRows, 2), movementDetectionFrame, ROIml,cameraFrame)&&multiplexingTask==false) {volume=changeVolume(volume-volumeSensibility);
                multiplexingTask=true;putText(cameraFrame, " Volume DOWN ", cvPoint(frameWidth-WidthOfROI,frameHeight-HeightOfROI/2),FONT_HERSHEY_COMPLEX_SMALL, 1.0, cvScalar(200,200,250), 1, CV_AA);}
            
            //lower right and upper left
            if (selectingRegionOfInterestWithDrawing(upLeft,xPositionOfROI(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true,1), yPositionOfROI(obstructionIndex, coordX, coordY, frameHeight, numberOfRows,1), widthOfRoi(obstructionIndex, coordX, coordY, frameWidth, numberOfColumns, true, 1), heightOfRoi(obstructionIndex, coordX, coordY, frameHeight, numberOfRows, 1), movementDetectionFrame, ROIul,cameraFrame)&&multiplexingTask==false) {volume=changeVolume(volume+volumeSensibility);putText(cameraFrame, " Volume UP  ", cvPoint(frameWidth-WidthOfROI,frameHeight-HeightOfROI/2),FONT_HERSHEY_COMPLEX_SMALL, 1.0, cvScalar(200,200,250), 1, CV_AA);;
                multiplexingTask=true;}
        }
        
        if (multiplexingCounter>20) {
            multiplexingCounter=0;
            multiplexing=false;
            multiplexingTask=false;
            
        }
        
        //Display
        imshow("iSight", cameraFrame);
        //detectAndDisplay(cameraFrame);
        //imshow("Even Frame", evenCameraFrame);
        //imshow("Odd Frame", oddCameraFrame);
        
    }
    
    return 0;
}
