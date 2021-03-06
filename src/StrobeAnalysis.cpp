/*
 ~ author: dviid
 ~ contact: dviid@labs.ciid.dk
 */

#include "StrobeAnalysis.h"
#include "ofMain.h"

#include "Poco/Timer.h"
#include "Poco/Thread.h"

#include "RefractiveIndex.h"

using Poco::Timer;
using Poco::TimerCallback;
using Poco::Thread;

void StrobeAnalysis::setup(int camWidth, int camHeight)
{
    DELTA_T_SAVE = 100;
    NUM_RUN = 1;
    
    _strobe_cnt = 0;
    _strobe_cnt_max = 20;  // 40 x 500 ms = 20000 ms = 20 seconds run time
    _strobe_interval = 1000;  //every 0.5seconds = 15 frames
    _frame_cnt_max = _strobe_cnt_max * _strobe_interval * ofGetFrameRate()/1000;
    
    // The British Health and Safety Executive recommend that a net flash rate for a bank of strobe lights does not exceed 5 flashes per second, at which only 5% of photosensitive epileptics are at risk. It also recommends that no strobing effect continue for more than 30 seconds, due to the potential for discomfort and disorientation.
    
    //or 20 times, every one second... 
    _save_cnt_max = _strobe_cnt_max*_strobe_interval/DELTA_T_SAVE;
    
    create_dir();
}


void StrobeAnalysis::acquire()
{

    Timer* save_timer;

    TimerCallback<StrobeAnalysis> save_callback(*this, &StrobeAnalysis::save_cb);

    // RUN ROUTINE
    for(int i = 0; i < NUM_RUN; i++) {

        _run_cnt = i;
        _save_cnt = 0;
        _frame_cnt = 0;

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

void StrobeAnalysis::synthesise()
{
    //incrementer to whichMesh
    speed=0.2;
    //whichMesh is the index in the vector of meshes
    whichMesh=0;
    
    int index=0;
    float iterator=1;
    bool debug=false;
    if(debug){
        _saved_filenames.clear();
        _saved_filenames=getListOfImageFilePaths("MIDDLESBOROUGH", _name);
        
        //hack to limit number of meshes.
        if(_saved_filenames.size()>100){
            iterator= _saved_filenames.size() /100;
        }
        
    }
    //clear vector so we don't add to it on successive runs
    meshes.clear();
    
    for(float i=0;i<_saved_filenames.size()-1;i+=iterator){
        
        
        ofImage image1;
        ofImage image2;
        
        //there is a known issue with using loadImage inside classes in other directories. the fix is to call setUseTExture(false)
        image1.setUseTexture(false);
        image2.setUseTexture(false);
        //some of the textures are not loading correctly so only make mesh if both the images load
        if(image1.loadImage(_saved_filenames[i]) && image2.loadImage(_saved_filenames[i+1])){
            meshes.push_back(ofMesh());
            cout<<"setting mesh"<<endl;
            int _recorded_brightness_value=getRecordedValueFromFileName(_saved_filenames[i]);
            setMeshFromPixels( calculateListOfZValues(image1,image2, COMPARE_BRIGHTNESS,_recorded_brightness_value), image2, &meshes[index]);            
            index++;
        }
    }
    
}

void StrobeAnalysis::display_results(){
    
    Timer* display_results_timer;
    
    TimerCallback<StrobeAnalysis> display_results_callback(*this, &StrobeAnalysis::display_results_cb);
    // display results of the synthesis
    
    display_results_timer = new Timer(0, 20); // timing interval for saving files
    display_results_timer->start(display_results_callback);
    _RUN_DONE = false;
    _results_cnt=0;
    _results_cnt_max=300;
    
    while(!_RUN_DONE)
        Thread::sleep(3);
    
    display_results_timer->stop();
    
}

// this runs at frame rate = 33 ms for 30 FPS
void StrobeAnalysis::draw()
{
    
    switch (_state) {
        case STATE_ACQUIRING:
        {
            
            if (_frame_cnt < _frame_cnt_max)
            {
                ofEnableAlphaBlending();
                ofColor aColour;
                int _fade_in_frames = _frame_cnt_max/10;
                cout<< "_fade_in_frames" << _fade_in_frames<< endl;
                
                if (_frame_cnt < _fade_in_frames) {
                    
                    aColour.set(255, 255, 255, ofMap(_frame_cnt, 0, _fade_in_frames, 0, 255));
                    ofSetColor(aColour);
                    ofRect(0, 0, ofGetWidth(), ofGetHeight());
                    //cout <<  "FADE IN STROBE TIME " << endl;
                    
                    
                }
                
                if (_frame_cnt >= _fade_in_frames && _frame_cnt < (_frame_cnt_max-_fade_in_frames)){
                    
                    //cout <<  "_frame_cnt: " << _frame_cnt << endl;
                    //cout <<  "frame_cnt % 15: " << _frame_cnt%15 << endl;
                    //cout <<  "MAIN STROBE TIME " << endl;
                    
                    if (_frame_cnt%int(ofGetFrameRate()*_strobe_interval/1000) < (ofGetFrameRate()*_strobe_interval/1000)/2)
                    {
                        ofSetColor(255, 255, 255);
                        ofRect(0, 0, ofGetWidth(), ofGetHeight());
                        _strobe_cnt++;
                        _strobe_on = 1;
                    } else if (_frame_cnt%int(ofGetFrameRate()*_strobe_interval/1000) >= (ofGetFrameRate()*_strobe_interval/1000)/2)
                    {
                        ofSetColor(0, 0, 0);
                        ofRect(0, 0, ofGetWidth(), ofGetHeight());
                        _strobe_on = 0;
                    }
                    
                }
                
                if (_frame_cnt >= (_frame_cnt_max-_fade_in_frames) && _frame_cnt < _frame_cnt_max) {
                    aColour.set(255, 255, 255, 255-ofMap(_frame_cnt, 0, _fade_in_frames, 0, 255));
                    ofSetColor(aColour);
                    ofRect(0, 0, ofGetWidth(), ofGetHeight());
                    // cout <<  "FADE OUT STROBE TIME " << endl;
                }         
            
                ofDisableAlphaBlending();
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
            int imageWidth=640;
            int imageHeight =480;
            ofPushMatrix();
            ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
            ofRotateY(_results_cnt*0.3);
            //ofRotateX(90);
            //ofRotateZ(whichMesh);
            ofTranslate(-ofGetWidth()/2, -ofGetHeight()/2),-400;
            ofTranslate((ofGetWidth()/2)-(imageWidth/2),0,0 );
            
            meshes[whichMesh].drawVertices();
            ofPopMatrix();
            whichMesh+=speed;
            cout<<whichMesh<<" size of meshes "<<meshes.size()<<endl;
            if(whichMesh>meshes.size() -1 || whichMesh<0){
                speed*=-1;
                whichMesh+=speed;
                
            }

            break;
        }
            
            
        default:
            break;
    }

}

// this runs at save_cb timer rate = DELTA_T_SAVE
void StrobeAnalysis::save_cb(Timer& timer)
{
    string file_name = ofToString(_save_cnt,2)+"_"+ ofToString(_strobe_on) +"_"+ofToString(_run_cnt,2)+".jpg";
    saveimage(file_name);
    
    _save_cnt++;
    
    cout << "_save_cnt" << _save_cnt << endl;
    
    cout << "_save_cnt_max" << _save_cnt_max << endl;
    
    //TODO:  something fucked here with my calc of _save_cnt_max - new structure should fix it?   
    //if(_save_cnt >= _save_cnt_max-10) {
    //    _RUN_DONE = true;
    //}

}

void StrobeAnalysis::display_results_cb(Timer& timer){
    _results_cnt++;
    if (_results_cnt>_results_cnt_max) {
        _RUN_DONE=true;
    }
}


