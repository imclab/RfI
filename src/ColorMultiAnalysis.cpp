/*
 - copyright (c) 2011 Copenhagen Institute of Interaction Design (CIID)
 - all rights reserved.

 + redistribution and use in source and binary forms, with or without
 + modification, are permitted provided that the following conditions
 + are met:
 +  > redistributions of source code must retain the above copyright
 +    notice, this list of conditions and the following disclaimer.
 +  > redistributions in binary form must reproduce the above copyright
 +    notice, this list of conditions and the following disclaimer in
 +    the documentation and/or other materials provided with the
 +    distribution.

 + THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 + "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 + LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 + FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 + COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 + INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 + BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 + OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 + AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 + OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 + OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 + SUCH DAMAGE.

 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ~ author: dviid
 ~ contact: dviid@labs.ciid.dk
 */

#include "ColorMultiAnalysis.h"
#include "ofMain.h"

#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "RefractiveIndex.h"

using Poco::Timer;
using Poco::TimerCallback;
using Poco::Thread;

#define COMPARE_RED 1
#define COMPARE_BLUE 2
#define COMPARE_GREEN 3
#define  COMPARE_HUE 4
#define COMPARE_BRIGHTNESS 5

void ColorMultiAnalysis::setup(int camWidth, int camHeight)
{
    DELTA_T_SAVE = 50;//150; // the right number is about 300
    NUM_PHASE = 1;
    NUM_RUN = 1;
    NUM_SAVE_PER_RUN = 100;//;    
    
    create_dir();
    _frame_cnt = 0;
    _fade_cnt=0;
    _frame_cnt_max = ofGetFrameRate() * ((DELTA_T_SAVE * NUM_SAVE_PER_RUN) / 1000);
    c = 0;
}

void ColorMultiAnalysis::acquire()
{

    Timer* save_timer;

    TimerCallback<ColorMultiAnalysis> save_callback(*this, &ColorMultiAnalysis::save_cb);

    // RUN ROUTINE
    for(int i = 0; i < NUM_RUN; i++) {
             
        _run_cnt = i;

        cout << "RUN NUM = " << i;

        save_timer = new Timer(0, DELTA_T_SAVE); // timing interval for saving files
        save_timer->start(save_callback);
        _RUN_DONE = false;
        _frame_cnt = 0; _save_cnt = 0;

        while(!_RUN_DONE)
            Thread::sleep(3);

        save_timer->stop();
    }

}

void ColorMultiAnalysis::synthesise()
{
    cout<<"SYNTHESISING MULTI";
    // _saved_filenames has all the file names of all the saved images
    // _saved_filenames has all the file names of all the saved images
    //incrementer to whichMesh
    speed=0.2;
    //whichMesh is the index in the vector of meshes
    whichMesh=0;

    cout<<"image loaded ";
    //there is a problem with natural vs alphabetical order when loading the files - we need to make a fix for this
    int shift=0;
    cout<<_saved_filenames.size()<<" image filenames ";
    int index=0;
    //for(int i=shift;i<_saved_filenames.size()-2;i+=2){
       // cout<<_saved_filenames[i]<<endl;

        meshes.push_back(ofMesh());
    image1.loadImage("/Users/tomschofield/of_preRelease_v007_osx/apps/refracitveGitRepoFeb/RefractiveIndex/src/macro.png");
        /*ofImage image2;
        image2.loadImage(_saved_filenames[i+1]);
        
        setMeshFromPixels( calculateListOfZValues(image1,image2, COMPARE_BLUE), image2, &meshes[index]);            
        */
    index++;
    //}
}


void ColorMultiAnalysis::draw()
{
    
    switch (_state) {
        case STATE_ACQUIRING:
        {
            
            if (_frame_cnt < _frame_cnt_max)
            {

                
                int _fade_in_frames = _frame_cnt_max/50;
                
                if (_frame_cnt < _fade_in_frames) {
                    ofColor aColor;
                    
                    aColor.setHsb(c, ofMap(_frame_cnt, 0, _fade_in_frames, 0, 255), ofMap(_frame_cnt, 0, _fade_in_frames, 0, 255));
                    
                    ofSetColor(aColor);
                    ofRect(0, 0, ofGetWidth(), ofGetHeight());
                    
                    cout << "FADING IN..." << endl;
                }
                
                if (_frame_cnt >= _fade_in_frames && _frame_cnt < _frame_cnt_max-_fade_in_frames){
                    
                    ofColor aColor;
                    aColor.setHsb(c, 255, 255);
                    ofSetColor(aColor);
                    
                    //how far are we as a percent of _frame_count_max * 360 HUE VALUES
                    c  = 255.0 * (_frame_cnt_max - _frame_cnt)/(_frame_cnt_max);
                    
                    ofRect(0, 0, ofGetWidth(), ofGetHeight());
                }
                
                if (_frame_cnt >= (_frame_cnt_max-_fade_in_frames) && _frame_cnt < _frame_cnt_max) {
                    
                    ofColor aColor;
                    
                    aColor.setHsb(c, 255-ofMap(_fade_cnt, 0, _fade_in_frames, 0, 255), 255-ofMap(_fade_cnt, 0, _fade_in_frames, 0, 255));
                    
                    ofSetColor(aColor);
                    ofRect(0, 0, ofGetWidth(), ofGetHeight());
                    
                    _fade_cnt++;
                    cout << "FADING OUT..." << endl;
                    
                }
                
            } else {
                 
                _RUN_DONE = true;
            
            }
            
            _frame_cnt++;            
            
            break;
        }
            
        case STATE_SYNTHESISING:
        {
            // display animation of something while the synthesis in on-going...
            break;
        }
            
        case STATE_DISPLAY_RESULTS:
        {
            // display results of the synthesis
            break;
        }
            
            
        default:
            break;
    }
    
}


// this runs at save_cb timer rate = DELTA_T_SAVE
void ColorMultiAnalysis::save_cb(Timer& timer)
{
    _save_cnt++;
    
    // UPDATE THE COLOR ON THE SCREEN
    //float c_last = c;
    
    // cout << "COLORMULTIANALYSIS::saving...\n";
    // cout << "c_last... " << c << endl;
    string file_name = ofToString(_save_cnt,2)+"_"+ofToString(c,2)+"_"+ofToString(_run_cnt,2)+".jpg";
    
    // cout<<_whole_file_path<<endl;
    ofSaveImage(RefractiveIndex::_pixels, _whole_file_path+"/"+file_name, OF_IMAGE_QUALITY_BEST);
<<<<<<< HEAD
   // _saved_filenames.push_back("/Users/tomschofield/of_preRelease_v007_osx/apps/myApps/refractiveIndexDavidFeb/bin/data/"+_whole_file_path+"/"+file_name);
    _saved_filenames.push_back("fish.jpg");

    if(_save_cnt >= NUM_SAVE_PER_RUN){
        _RUN_DONE = true;
    }
=======
    
    //if(_save_cnt >= NUM_SAVE_PER_RUN){
    //    _RUN_DONE = true;
    //}
>>>>>>> 88fa0375934e9ad87053542e88a0f9fe61af0a66
}
