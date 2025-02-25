
#include "MainComponent.h"
#include "Utils.h"
#include <boost/filesystem.hpp>

#define offset 75

MainContentComponent::MainContentComponent():
oscHandler(),
clippingLed( *this ),
//overLed( *this ),
audioIOComponent(),
audioRecorder(),
delayLine(),
sourceImagesHandler(),
ambi2binContainer()
{
    // set window dimensions
    setSize (650, 900);
    
    // specify the required number of input and output channels
    setAudioChannels (2, 2);
    
    // add to change listeners
    oscHandler.addChangeListener(this);
    
    // add audioIOComponent as addAudioCallback for adc input
    deviceManager.addAudioCallback(&audioIOComponent);

    // set presetPath
    presetPath = boost::filesystem::path("~/Documents/auralizer/presets/");
    
    //==========================================================================
    // INIT GUI ELEMENTS
    
    // add GUI sub-components
    addAndMakeVisible(audioIOComponent);
    addAndMakeVisible(clippingLed);
    clippingLed.setAlwaysOnTop(true);
//    addAndMakeVisible(overLed);
//    overLed.setAlwaysOnTop(true);




    // new GUI elements
    addAndMakeVisible(pressureSlider); 
    addAndMakeVisible(tempSlider);
    addAndMakeVisible(humiditySlider);
    addAndMakeVisible(medSlider);


    
    // setup logo image
//    logoImage = ImageCache::getFromMemory(BinaryData::evertims_logo_512_png, BinaryData::evertims_logo_512_pngSize);
//    logoImage = logoImage.rescaled(logoImage.getWidth()/2, logoImage.getHeight()/2);

    // init log text box
    addAndMakeVisible (logTextBox);
    logTextBox.setMultiLine (true);
    logTextBox.setReturnKeyStartsNewLine (true);
    logTextBox.setReadOnly (true);
    logTextBox.setScrollbarsShown (true);
    logTextBox.setCaretVisible (false);
    logTextBox.setPopupMenuEnabled (true);
    logTextBox.setColour (TextEditor::textColourId, Colours::whitesmoke);
    logTextBox.setColour (TextEditor::backgroundColourId, Colour(PixelARGB(200,30,30,30)));
    logTextBox.setColour (TextEditor::outlineColourId, Colours::whitesmoke);
    logTextBox.setColour (TextEditor::shadowColourId, Colours::darkorange);
    
    // init text buttons
    buttonMap.insert({
        { &saveIrButton, "Save RIRs to Desktop" },
        { &saveOscButton, "Save OSC state to Desktop" },
        { &clearSourceImageButton, "Clear" },
        { &saveToHost, "Save to Host"}
    });
    for (auto& pair : buttonMap)
    {
        auto& obj = pair.first;
        const auto& param = pair.second;
        obj->setButtonText(param);
        obj->addListener (this);
        obj->setEnabled (true);
        addAndMakeVisible(obj);
    }
    saveIrButton.setColour (TextButton::buttonColourId, Colours::transparentBlack);
    saveOscButton.setColour (TextButton::buttonColourId, Colour(PixelARGB(160,0,0,0)));
    clearSourceImageButton.setColour (TextButton::buttonColourId, Colours::indianred);
    saveToHost.setColour(TextButton::buttonColourId, Colours::transparentBlack);


    // init combo boxes
    comboBoxMap.insert({
        { &numFrequencyBandsComboBox, {"3", "10"} },
        { &srcDirectivityComboBox, {"omni", "directional"} },
    });
    for (auto& pair : comboBoxMap)
    {
        auto& obj = pair.first;
        const auto& param = pair.second;
        addAndMakeVisible(obj);
        obj->setEditableText(false);
        obj->setJustificationType(Justification::right);
        obj->setColour(ComboBox::backgroundColourId, Colour(PixelARGB(200,30,30,30)));
        obj->setColour(ComboBox::buttonColourId, Colour(PixelARGB(200,30,30,30)));
        obj->setColour(ComboBox::outlineColourId, Colour(PixelARGB(200,30,30,30)));
        obj->setColour(ComboBox::textColourId, Colours::whitesmoke);
        obj->setColour(ComboBox::arrowColourId, Colours::whitesmoke);
        obj->addListener (this);
        for( int i = 0; i < param.size(); i++ ){ obj->addItem(param[i], i+1); }
        obj->setSelectedId(1);
    }
    
    // init sliders
    sliderMap.insert({
        { &gainReverbTailSlider, { 0.0, 2.0, 1.0} }, // min, max, value
        { &gainDirectPathSlider, { 0.0, 2.0, 1.0} },
        { &gainEarlySlider, { 0.0, 4.0, 1.0} },
        { &crossfadeStepSlider, { 0.001, 0.2, 0.1} },
        { &medSlider, {0, 1, 0} },
        { &tempSlider, {0.0f, 1.0f, 0.1f} },
        { &pressureSlider, {0.0f, 1.0f, 0.1f} },
        { &humiditySlider, {0.0f, 1.0f, 0.1f} }
    });
    gainReverbTailSlider.setRange(0.0, 2.0, 0.01);
    gainDirectPathSlider.setRange(0.0, 2.0, 0.01);
    gainEarlySlider.setRange(0.0, 2.0, 0.01);
    crossfadeStepSlider.setRange(0.001, 0.2, 0.001);

    for (auto& pair : sliderMap)
    {
        auto& obj = pair.first;
        const auto& param = pair.second;
        addAndMakeVisible(obj);
        obj->setRange( param[0], param[1] );
        obj->setValue( param[2] );
        obj->setSliderStyle(Slider::LinearHorizontal);
        obj->setColour(Slider::textBoxBackgroundColourId, Colours::transparentBlack);
        obj->setColour(Slider::backgroundColourId, Colours::darkgrey);
        obj->setColour(Slider::trackColourId, Colours::lightgrey);
        obj->setColour(Slider::thumbColourId, Colours::white);
        obj->setColour(Slider::textBoxTextColourId, Colours::white);
        obj->setColour(Slider::textBoxOutlineColourId, Colours::transparentBlack);
        obj->setTextBoxStyle(Slider::TextBoxRight, true, 70, 20);
        obj->addListener(this);
    }
    crossfadeStepSlider.setSliderStyle(Slider::RotaryVerticalDrag);
    crossfadeStepSlider.setColour(Slider::rotarySliderFillColourId, Colours::white);
    crossfadeStepSlider.setColour(Slider::rotarySliderOutlineColourId, Colours::darkgrey);
    crossfadeStepSlider.setTextBoxStyle(Slider::TextBoxRight, true, 50, 20);
    crossfadeStepSlider.setRotaryParameters(10 / 8.f * 3.1416, 22 / 8.f * 3.1416, true);
    crossfadeStepSlider.setSkewFactor(0.7);

    // make sliders horizontal bar type
    pressureSlider.setSliderStyle(Slider::LinearHorizontal);
    humiditySlider.setSliderStyle(Slider::LinearHorizontal);
    tempSlider.setSliderStyle(Slider::LinearHorizontal);
    medSlider.setSliderStyle(Slider::LinearHorizontal);

    pressureSlider.setRange(-10.0f, 10.0f, 0.01);
    pressureSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    pressureSlider.setPopupDisplayEnabled(true, false, this);
    pressureSlider.setTextValueSuffix(" atm");

    humiditySlider.setRange(0.0f, 200.0f, 0.1);
    humiditySlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    humiditySlider.setPopupDisplayEnabled(true, false, this);
    humiditySlider.setTextValueSuffix(" %");
    humiditySlider.setValue(50.0f);

    tempSlider.setRange(-273.15f, 2731500.0f, 0.1);
    tempSlider.setSkewFactorFromMidPoint(20.0f);
    tempSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    tempSlider.setPopupDisplayEnabled(true, false, this);
    tempSlider.setTextValueSuffix(" C");
    tempSlider.setValue(20.0f);

    medSlider.setRange(0, 1, 1);
    medSlider.setValue(0);

    // init labels
    labelMap.insert({
        { &numFrequencyBandsLabel, "Num absorb freq bands:" },
        { &srcDirectivityLabel, "Source directivity:" },
        { &inputLabel, "Inputs" },
        { &parameterLabel, "Parameters" },
        { &logLabel, "Logs - build 0.2" },
        { &directPathLabel, "Direct path" },
        { &earlyLabel, "Early reflections" },
        { &crossfadeLabel, "Crossfade factor" },
        { &clippingLedLabel, "clip" },
        // new labels
        { &medSliderLabel, "medium" },
        { &tempSliderLabel, "Temperature" },
        { &pressureSliderLabel, "Pressure" },
        { &humiditySliderLabel, "% Humidity" }
    });
    for (auto& pair : labelMap)
    {
        auto& obj = pair.first;
        const auto& param = pair.second;
        addAndMakeVisible(obj);
        obj->setText( param, dontSendNotification );
        obj->setColour(Label::textColourId, Colours::whitesmoke);
        
    }
    inputLabel.setColour(Label::backgroundColourId, Colour(30, 30, 30));
    parameterLabel.setColour(Label::backgroundColourId, Colour(30, 30, 30));
    logLabel.setColour(Label::backgroundColourId, Colour(30, 30, 30));
    
    // init toggles
    toggleMap.insert({
        { &reverbTailToggle, "Reverb tail" },
        { &enableDirectToBinaural, "Direct to binaural" },
        { &enableLog, "Enable logs" },
        { &enableRecord, "Record Ambisonic to disk" }
    });
    for (auto& pair : toggleMap)
    {
        auto& obj = pair.first;
        const auto& param = pair.second;
        addAndMakeVisible(obj);
        obj->setButtonText( param );
        obj->setColour(ToggleButton::textColourId, Colours::whitesmoke);
        obj->setEnabled(true);
        obj->addListener(this);
        if( obj != &enableRecord ){
            obj->setToggleState(true, juce::sendNotification);
        }
    }
    
    // disable direct to binaural until fixed
    // enableDirectToBinaural.setEnabled(false);
    enableDirectToBinaural.setToggleState(false, juce::sendNotification);
    enableRecord.setToggleState(false, juce::sendNotification);
}

MainContentComponent::~MainContentComponent()
{
    // fix denied access at close when sound playing,
    // see https://forum.juce.com/t/tutorial-playing-sound-files-raises-an-exception-on-2nd-load/15738/2
    audioIOComponent.transportSource.setSource(nullptr);
    
    shutdownAudio();
}

//==============================================================================
void MainContentComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.
    // Called on the audio thread, not the GUI thread.
    AirAbsorber.setNumBands(numFreqBands);
    AirAbsorber.setHumidity(humiditySlider.getValue());
    AirAbsorber.setPressure(pressureSlider.getValue());
    AirAbsorber.setMedium(0);
    AirAbsorber.setTemperature(tempSlider.getValue()); 
    
    // audio file reader & adc input
    audioIOComponent.prepareToPlay (samplesPerBlockExpected, sampleRate);
    
    // recorder
    audioRecorder.prepareToPlay (samplesPerBlockExpected, sampleRate);
    
    // working buffer
    workingBuffer.setSize(1, samplesPerBlockExpected);
    // ambisonic buffer holds 2 stereo channels (first) + ambisonic channels
    ambisonicBuffer.setSize(2 + N_AMBI_CH, samplesPerBlockExpected);
    // because of stupid design choice of ambisonicBuffer, require this additional ambisonicRecordBuffer. to clean..
    ambisonicRecordBuffer.setSize(N_AMBI_CH, samplesPerBlockExpected);
    
    // keep track of sample rate
    localSampleRate = sampleRate;
    localSamplesPerBlockExpected = samplesPerBlockExpected;
    
    // init delay line
    delayLine.prepareToPlay(samplesPerBlockExpected, sampleRate);
    delayLine.setSize(1, sampleRate); // arbitrary length of 1 sec
    sourceImagesHandler.prepareToPlay (samplesPerBlockExpected, sampleRate);
    
    // init ambi 2 bin decoding: fill in data in ABIR filtered and ABIR filter themselves
    for( int i = 0; i < N_AMBI_CH; i++ )
    {
        ambi2binFilters[ 2*i ].init(samplesPerBlockExpected, AMBI2BIN_IR_LENGTH);
        ambi2binFilters[ 2*i ].setImpulseResponse(ambi2binContainer.ambi2binIrDict[i][0].data()); // [ch x ear x sampID]
        ambi2binFilters[2*i+1].init(samplesPerBlockExpected, AMBI2BIN_IR_LENGTH);
        ambi2binFilters[2*i+1].setImpulseResponse(ambi2binContainer.ambi2binIrDict[i][1].data()); // [ch x ear x sampID]
    }
}

// Audio Processing (split in "processAmbisonicBuffer" and "fillNextAudioBlock" to enable
// IR recording: using the same methods as the main thread)
void MainContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // check if update required
    if( updateNumFreqBandrequired ){
        sourceImagesHandler.setFilterBankSize(numFreqBands);
        updateNumFreqBandrequired = false;
        // trigger general update: must re-dimension abs.coeffs and trigger update future->current, see in function
        sourceImagesHandler.updateFromOscHandler(oscHandler);
    }
    
    // fill buffer with audiofile data
    audioIOComponent.getNextAudioBlock(bufferToFill);
    
    // execute main audio processing
    if( !isRecordingIr )
    {
        processAmbisonicBuffer( bufferToFill.buffer );
        if( audioRecorder.isRecording() ){recordAmbisonicBuffer(); }
        fillNextAudioBlock( bufferToFill.buffer);
    }
    // simply clear output buffer
    else
    {
        bufferToFill.clearActiveBufferRegion();
    }
    
    // check if source images need update (i.e. update called by OSC handler
    // while source images in the midst of a crossfade
    if( sourceImageHandlerNeedsUpdate && sourceImagesHandler.crossfadeOver )
    {
        sourceImagesHandler.updateFromOscHandler(oscHandler);
        requireDelayLineSizeUpdate = true;
        sourceImageHandlerNeedsUpdate = false;
    }
}

// Audio Processing: split from getNextAudioBlock to use it for recording IR
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainContentComponent::processAmbisonicBuffer( AudioBuffer<float> *const audioBufferToFill )
{
    workingBuffer.copyFrom(0, 0, audioBufferToFill->getWritePointer(0), workingBuffer.getNumSamples());
    
    //==========================================================================
    // SOURCE IMAGE PROCESSING
    
    if ( sourceImagesHandler.numSourceImages > 0 )
    {
        
        //==========================================================================
        // DELAY LINE
        
        // update delay line size if need be (TODO: MOVE THIS .SIZE() OUTSIDE OF AUDIO PROCESSING LOOP
        if ( requireDelayLineSizeUpdate )
        {
            // get maximum required delay line duration
            float maxDelay = sourceImagesHandler.getMaxDelayFuture();
            
            // get associated required delay line buffer length
            int updatedDelayLineLength = (int)( 1.5 * maxDelay * localSampleRate); // longest delay creates noisy sound if delay line is exactly 1* its duration
            
            // update delay line size
            delayLine.setSize(1, updatedDelayLineLength);
            
            // unflag update required
            requireDelayLineSizeUpdate = false;
        }
        
        // add current audio buffer to delay line
        delayLine.copyFrom(0, workingBuffer, 0, 0, workingBuffer.getNumSamples());
        
        // loop over sources images, apply delay + room coloration + spatialization
        sourceImagesHandler.getNextAudioBlock( & delayLine, ambisonicBuffer );
        
        // increment delay line write position
        delayLine.incrementWritePosition(workingBuffer.getNumSamples());
        
    }
    
}

void MainContentComponent::fillNextAudioBlock( AudioBuffer<float> *const audioBufferToFill )
{
    
    //==========================================================================
    // SPATIALISATION: Ambisonic decoding + virtual speaker approach + binaural
    
    if ( sourceImagesHandler.numSourceImages > 0 )
    {
        // duplicate channel before filtering for two ears
        ambisonicBuffer2ndEar = ambisonicBuffer;

        // loop over Ambisonic channels
        for (int k = 0; k < N_AMBI_CH; k++)
        {
            ambi2binFilters[ 2*k ].process(ambisonicBuffer.getWritePointer(k+2)); // left
            ambi2binFilters[2*k+1].process(ambisonicBuffer2ndEar.getWritePointer(k+2)); // right
            
            // collapse left channel, collapse right channel
            ambisonicBuffer.addFrom(0, 0, ambisonicBuffer.getWritePointer(2+k), workingBuffer.getNumSamples());
            ambisonicBuffer2ndEar.addFrom(1, 0, ambisonicBuffer2ndEar.getWritePointer(2+k), workingBuffer.getNumSamples());
        }

        // final rewrite to output buffer
        audioBufferToFill->copyFrom(0, 0, ambisonicBuffer, 0, 0, workingBuffer.getNumSamples());
        audioBufferToFill->copyFrom(1, 0, ambisonicBuffer2ndEar, 1, 0, workingBuffer.getNumSamples());
    }
    
    //==========================================================================
    // if no source image, simply rewrite to output buffer (TODO: remove stupid double copy)
    else
    {
        audioBufferToFill->copyFrom(0, 0, workingBuffer, 0, 0, workingBuffer.getNumSamples());
        audioBufferToFill->copyFrom(1, 0, workingBuffer, 0, 0, workingBuffer.getNumSamples());
    }
    
    //==========================================================================
    // CLIP OUTPUT (DEBUG PRECAUTION)
    auto outL = audioBufferToFill->getWritePointer(0);
    auto outR = audioBufferToFill->getWritePointer(1);
    for (int i = 0; i < workingBuffer.getNumSamples(); i++)
    {
        outL[i] = clipOutput(outL[i]);
        outR[i] = clipOutput(outR[i]);
    }
}

// record Ambisonic buffer to disk
void MainContentComponent::recordAmbisonicBuffer()
{
    if ( sourceImagesHandler.numSourceImages > 0 )
    {
        // loop over Ambisonic channels to extract only ambisonic channels. I know, stupid. Needs cleaning
        for (int k = 0; k < N_AMBI_CH; k++)
        {
            ambisonicRecordBuffer.copyFrom(k, 0, ambisonicBuffer, k+2, 0, ambisonicBuffer.getNumSamples());
        }
    }
    // if no source image, data in ambisonicBuffer is meaningless: copy content of workingbuffer (raw input) rather
    else{
        ambisonicRecordBuffer.clear();
        ambisonicRecordBuffer.copyFrom(0, 0, workingBuffer, 0, 0, ambisonicBuffer.getNumSamples());
    }
    
    // write to disk
    audioRecorder.recordBuffer((const float **) ambisonicRecordBuffer.getArrayOfWritePointers(), ambisonicRecordBuffer.getNumChannels(), ambisonicRecordBuffer.getNumSamples());
}

// record current Room impulse Response to disk
void MainContentComponent::recordIr(boost::filesystem::path path, juce::String name) // NOTE: RECORD_IR
{
    /*
    string path:
        the path to record the IR to
     string name:
        the name

     */

    // estimate output buffer size (based on max delay time)
    auto maxDelaySourceImages = getMaxValue( oscHandler.getSourceImageDelays() );
    auto rt60 = oscHandler.getRT60Values();
    float maxDelayRt60 = getMaxValue(rt60);
    float maxDelay = fmax( maxDelaySourceImages, maxDelayRt60 );
    int maxDelayInSamp = ceil(maxDelay * localSampleRate);
    // output buffer at least 1 localSamplesPerBlockExpected long
    maxDelayInSamp = fmax( maxDelayInSamp, localSamplesPerBlockExpected);
    
    // get min delay
    int minDelayInSamp = ceil( localSampleRate * getMinValue( oscHandler.getSourceImageDelays() ) );
    
    // init
    recordingBufferInput.setSize(2, localSamplesPerBlockExpected);
    recordingBufferInput.clear();
    recordingBufferOutput.setSize(2, 2*maxDelayInSamp);
    recordingBufferOutput.clear();
    recordingBufferAmbisonicOutput.setSize(N_AMBI_CH, 2*maxDelayInSamp);
    recordingBufferAmbisonicOutput.clear();
    
    // prepare impulse response buffer
    recordingBufferInput.getWritePointer(0)[0] = 1.0f;
    
    // clear delay lines / fdn buffers of main thread
    delayLine.clear();
    sourceImagesHandler.reverbTail.clear();
    
    // pass impulse input into processing loop until IR faded below threshold
    float rms = 1.0f;
    int bufferId = 0;
    float irRmsThreshold = 0.0f;
    // record until rms below threshold or reached max delay. minDelay used here to make sure recording doesn't stop on first buffers for large
    // source-listener distances, where RMS is zero for the first few buffers until LOS image source reaches listener.
    while( ( rms >= irRmsThreshold || bufferId*localSamplesPerBlockExpected < minDelayInSamp ) && bufferId*localSamplesPerBlockExpected < maxDelayInSamp )
    {
        // clear impulse after first round
        if( bufferId >= 1 ){ recordingBufferInput.clear(); }
        
        // execute main audio processing: fill ambisonic buffer
        processAmbisonicBuffer( &recordingBufferInput );
        
        // add to output ambisonic buffer
        for( int k = 0; k < N_AMBI_CH; k++ )
        {
            recordingBufferAmbisonicOutput.addFrom(k, bufferId*localSamplesPerBlockExpected, ambisonicBuffer, k+2, 0, localSamplesPerBlockExpected);
        }
        
        // ambisonic to stereo
        fillNextAudioBlock( &recordingBufferInput );
        
        // add to output stereo buffer
        for( int k = 0; k < 2; k++ )
        {
            recordingBufferOutput.addFrom(k, bufferId*localSamplesPerBlockExpected, recordingBufferInput, k, 0, localSamplesPerBlockExpected);
        }
        
        // increment
        bufferId += 1;
        rms = recordingBufferInput.getRMSLevel(0, 0, localSamplesPerBlockExpected) + recordingBufferInput.getRMSLevel(0, 0, localSamplesPerBlockExpected);
        rms *= 0.5;
    }
    
    // resize output IR buffers to max meaningful sample length
    recordingBufferOutput.setSize(2, bufferId*localSamplesPerBlockExpected, true);
    recordingBufferAmbisonicOutput.setSize(N_AMBI_CH, bufferId*localSamplesPerBlockExpected, true);
    
    // save output

    if (name == "")
    {
        audioIOComponent.saveIR(recordingBufferAmbisonicOutput, localSampleRate, nullptr, String("Evertims_IR_Recording_ambi_") + String(AMBI_ORDER) + String("_order"));
        audioIOComponent.saveIR(recordingBufferOutput, localSampleRate, nullptr, "Evertims_IR_Recording_binaural");
    }
    else
    {

        audioIOComponent.saveIR(recordingBufferAmbisonicOutput, localSampleRate, &path, name);
    }


}

void MainContentComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.
    
    audioIOComponent.transportSource.releaseResources();
    
    // clear all "delay line" like buffers
    delayLine.clear();
    sourceImagesHandler.reverbTail.clear();
}


//==============================================================================
// ADDITIONAL AUDIO ROUTINES

// CRUDE DEBUG PRECAUTION - clip output in [-1.0; 1.0]
float MainContentComponent::clipOutput(float input)
{
    if (std::abs(input) > 1.0f)
    {
        clippingLed.isClipped = true;
        return sign(input)*fmin(std::abs(input), 1.0f);
    }
    else
        return input;
}

// method called when new OSC messages are available
void MainContentComponent::updateOnOscReceive()
{
    // update OSC handler internals (has to happend here, to ensure that they won't be updated while
    // running the sourceImagesHandler.updateFromOscHandler method that reads them OSC internals.
    oscHandler.updateInternals();
    
    // if sourceImagesHandler not in the midst of an update
    if( sourceImagesHandler.crossfadeOver && !sourceImageHandlerNeedsUpdate )
    {
        // update source images attributes based on latest received OSC info
        sourceImagesHandler.updateFromOscHandler(oscHandler);
        
        // now that everything is ready: set update flag, to resize delay line at next audio loop
        requireDelayLineSizeUpdate = true;
    }
    // otherwise, flag that an update is required
    else{ sourceImageHandlerNeedsUpdate = true; }
}

//==============================================================================
// GRAPHIC METHODS

void MainContentComponent::paint (Graphics& g)
{
    // background
    g.fillAll (Colour(PixelARGB(240,30,30,30)));
    
    // parameters box
    // g.setOpacity(1.0f);
    g.setColour(Colours::white);
    g.drawRect(10.f, 155.f, getWidth()-20.f, 330.f);

    g.drawLine(30, 240 + offset, getWidth() - 30, 240 + offset);
    
    // logo image
    g.drawImageAt(logoImage, (int)( (getWidth()/2) - (logoImage.getWidth()/2) ), 380);
    
    // signature
    g.setColour(Colours::white);
    g.setFont(11.f);
    g.drawFittedText("designed by D. Poirier-Quinot, M. Noisternig, and B. F.G. Katz (2017), this build modified by Z. L. Towbes (2019).", getWidth() - 335, getHeight()-15, 325, 15, Justification::right, 2);
}

void MainContentComponent::resized()
{
    // audio IO box
    inputLabel.setBounds(30, 3, 50, 15);
    audioIOComponent.setBounds(10, 10, getWidth()-20, 130);
    
    // parameters box
    int thirdWidthIoComponent = (int)((getWidth() - 20)/ 3) - 20; // lazy to change all to add that to audioIOComponent GUI for now

    parameterLabel.setBounds(30, 147, 80, 15);
    
    directPathLabel.setBounds(30, 170, 120, 20);
    gainDirectPathSlider.setBounds (140, 170, getWidth() - 320, 20);
    enableDirectToBinaural.setBounds ( getWidth() - 170, 170, 140, 20);

    earlyLabel.setBounds(30, 200, 120, 20);
    gainEarlySlider.setBounds (180, 200, getWidth() - 270, 20);

    reverbTailToggle.setBounds(30, 230, 120, 20);
    gainReverbTailSlider.setBounds (180, 230, getWidth() - 270, 20);

    clearSourceImageButton.setBounds(getWidth() - 80, 200, 50, 50);
    
    saveIrButton.setBounds(getWidth() - thirdWidthIoComponent - 30, 265, thirdWidthIoComponent, 30);
    saveOscButton.setBounds(getWidth() - thirdWidthIoComponent - 15, getHeight() - 75, thirdWidthIoComponent, 30);

    
    crossfadeStepSlider.setBounds(0.16*getWidth(), 255, 90, 50);
    crossfadeLabel.setBounds(30, 258, crossfadeStepSlider.getX() - 30, 40);
    
    numFrequencyBandsLabel.setBounds(190, 260, getWidth() - 450, 20);
    numFrequencyBandsComboBox.setBounds(saveIrButton.getX()- 70, 260, 70, 20);
    
    srcDirectivityLabel.setBounds(190, 280, getWidth() - 450, 20);
    srcDirectivityComboBox.setBounds(saveIrButton.getX() - 110, 280, 110, 20);
    
    // log box
//    logTextBox.setBounds(<#int x#>, <#int y#>, <#int width#>, <#int height#>)
    logLabel.setBounds(30, 489, 120, 20);
    logTextBox.setBounds (8, 500, getWidth() - 16, getHeight() - 536);
    enableLog.setBounds(getWidth() - 120, 510, 100, 30);
    enableRecord.setBounds(getWidth() - 200, 540, 180, 30);
    
    // clipping led
    clippingLedLabel.setBounds(enableLog.getX() - 50, enableLog.getY()+7, 34, 14);
    clippingLed.setBounds(clippingLedLabel.getX() - 12, enableLog.getY()+4, 16, 16);

    /*
     Slider humiditySlider;
     Slider pressureSlider;
     Slider tempSlider;
     Slider medSlider;
     */


    humiditySlider.setBounds(180, 260 + offset, getWidth() - 380, 20);
    humiditySliderLabel.setBounds(30, 260 + offset, 120, 20);

    pressureSlider.setBounds(180, 290 + offset, getWidth() - 380, 20);
    pressureSliderLabel.setBounds(30, 290 + offset, 120, 20);

    tempSlider.setBounds(180, 320 + offset, getWidth() - 380, 20);
    tempSliderLabel.setBounds(30, 320 + offset, 120, 20);

    medSlider.setBounds(180, 350 + offset, getWidth() - 380, 20);
    medSliderLabel.setBounds(30, 350 + offset, 120, 20);


    saveToHost.setBounds(getWidth() - thirdWidthIoComponent - 30, 370 + offset, thirdWidthIoComponent, 30);
}

//==============================================================================
// LISTENER METHODS

void MainContentComponent::changeListenerCallback (ChangeBroadcaster* broadcaster)
{
    if (broadcaster == &oscHandler)
    {
        if( enableLog.getToggleState() )
        {
            logTextBox.setText(oscHandler.getMapContentForGUI());
        }
        updateOnOscReceive();
    }
}

void MainContentComponent::buttonClicked (Button* button)
{
    if (button == &saveIrButton)
    {
        if ( sourceImagesHandler.numSourceImages > 0 )
        {
            isRecordingIr = true;
            recordIr("~/Desktop", ""); // this leaves name empty because if name == "", recordIR() will give it the default name.

            // unlock main audio thread
            isRecordingIr = false;
        }
        else {
            AlertWindow::showMessageBoxAsync ( AlertWindow::NoIcon, "Impulse Response not saved", "No source images registered from raytracing client \n(Empty IR)", "OK");
        }
    }
    if (button == &saveOscButton)
    {
        String output = oscHandler.getMapContentForLog();
        saveStringToDesktop("EVERTims_state", output);
    }
    if( button == &reverbTailToggle )
    {
        sourceImagesHandler.enableReverbTail = reverbTailToggle.getToggleState();
        updateOnOscReceive(); // require delay line size update
        gainReverbTailSlider.setEnabled(reverbTailToggle.getToggleState());
    }
    if( button == &enableDirectToBinaural )
    {
        sourceImagesHandler.enableDirectToBinaural = enableDirectToBinaural.getToggleState();
    }
    if( button == &enableLog )
    {
        if( button->getToggleState() ){ logTextBox.setText(oscHandler.getMapContentForGUI()); }
        else{ logTextBox.setText( String("") ); };
    }
    if( button == &enableRecord )
    {
        if( button->getToggleState() ){ audioRecorder.startRecording(); }
        else{ audioRecorder.stopRecording(); };
    }
    if( button == &clearSourceImageButton )
    {
        oscHandler.clear(false);
        updateOnOscReceive();
        logTextBox.setText(oscHandler.getMapContentForGUI());
    }
    if (button == &saveToHost){
        if ( sourceImagesHandler.numSourceImages > 0 )
        {
            isRecordingIr = true;
            saveAuralizerPreset(presetPath.string());
            // unlock main audio thread
            isRecordingIr = false;
        }
        else {
            AlertWindow::showMessageBoxAsync ( AlertWindow::NoIcon, "Error: Preset not saved", "No source images registered from raytracing client \n(Empty IR)", "OK");
        }
    }

}

void MainContentComponent::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox == &numFrequencyBandsComboBox)
    {
        // update locals
        if( numFrequencyBandsComboBox.getSelectedId() == 1 ) numFreqBands = 3;
        else numFreqBands = 10;
        
        // flag update required in audio loop (to avoid multi-thread access issues)
        updateNumFreqBandrequired = true;
    }
    if (comboBox == &srcDirectivityComboBox)
    {
        // load new file
        string filename;
        if( comboBox->getSelectedId() == 1 ) filename = "omni.sofa";
        else filename = "directional.sofa";
        const char *fileChar = filename.c_str();
        sourceImagesHandler.directivityHandler.loadFile( fileChar );
        
        // update 
        updateOnOscReceive();
    }
}

void MainContentComponent::sliderValueChanged(Slider* slider)
{
    if( slider == &gainReverbTailSlider )
    {
        sourceImagesHandler.reverbTailGain = gainReverbTailSlider.getValue();
    }
    if( slider == &gainDirectPathSlider )
    {
        sourceImagesHandler.directPathGain = gainDirectPathSlider.getValue();
    }
    if( slider == &gainEarlySlider )
    {
        sourceImagesHandler.earlyGain = slider->getValue();
    }
    if( slider == &crossfadeStepSlider )
    {
        sourceImagesHandler.crossfadeStep = slider->getValue();
        sourceImagesHandler.binauralEncoder.crossfadeStep = slider->getValue();
    }
    // new sliders
    if( slider == &pressureSlider )
    {
        AirAbsorber.setPressure(slider->getValue());
        update_air_coeffs(numFreqBands);
    }
    if( slider == &humiditySlider )
    {
        AirAbsorber.setHumidity(slider->getValue() / 100.0f);
        update_air_coeffs(numFreqBands);
    }
    if( slider == &tempSlider )
    {
        AirAbsorber.setTemperature(slider->getValue());
        update_air_coeffs(numFreqBands);
    }
    if( slider == &medSlider )
    {
        AirAbsorber.setMedium((int) slider->getValue());
        update_air_coeffs(numFreqBands);
    }

}

void MainContentComponent::update_air_coeffs(int numbands)
{
    // this might be poor thread safety, oh well. Sorry Akito.
    if (numbands == 3){
        for (int i = 0; i < numbands; i++){
            sourceImagesHandler.a_3[i] = AirAbsorber.getCoeffForBand(i, numFreqBands);
        }
    }
    else if (numbands == 10){
        for (int i = 0; i < numbands; i++){
            sourceImagesHandler.a_10[i] = AirAbsorber.getCoeffForBand(i, numFreqBands);
        }
    }
}

void MainContentComponent::saveAuralizerPreset(boost::filesystem::path presetPath){
    //=============================================================================//
    // SAVE THE XML PRESET ========================================================//
    //=============================================================================//

    std::unique_ptr<XmlElement> xml (new XmlElement(presetSaveName));

    boost::filesystem::path originalPresetPath(presetPath.string());

    xml->setAttribute("name", presetSaveName.toUTF8());
    xml->setAttribute("IRDir", presetPath.string());

    xml->setAttribute("wetAmt", 1.0f); // 1.0f is the default for wet
    xml->setAttribute("dryAmt", 0.0f); // 0.0f is the default for dry
    xml->setAttribute("inAmt", 1.0f); // ...
    xml->setAttribute("outAmt", 1.0f);
    xml->setAttribute("yawAmt", 0.0f);
    xml->setAttribute("pitchAmt", 0.0f);
    xml->setAttribute("distAmt", 1.0f);
    xml->setAttribute("dirAmt", (double) sourceImagesHandler.directPathGain);
    xml->setAttribute("earlyAmt", (double) sourceImagesHandler.earlyGain);
    xml->setAttribute("lateAmt", (double) sourceImagesHandler.reverbTailGain);

//    presetPath is the path to save presets to. It is normally "~/Documents/auralizer/presets/
    boost::filesystem::path thisPresetPath = boost::filesystem::path(presetPath.string());



    thisPresetPath /= presetSaveName.toUTF8();
    thisPresetPath /= "/";

    File createDirFile(thisPresetPath.string());
    createDirFile.createDirectory();

//    juce::String IRPathString = createDirFile.getFullPathName();
    boost::filesystem::path IRPath(createDirFile.getFullPathName().toUTF8());

    thisPresetPath /= presetSaveName.toUTF8(); // appends the savename to the preset path
    thisPresetPath.replace_extension(".xml"); // appends ".xml" to the preset's name

    File xmlFile(thisPresetPath.string());

    FileOutputStream xmlFileOutputStream(xmlFile);

    xml->writeTo(xmlFileOutputStream);
    

//    juce::File presetFile = juce::File()


    //=============================================================================//
    // SAVE THE IMPULSE RESPONSES OF THE PRESET ===================================//
    //=============================================================================//

    // save the current slider positions so that they can be reset correctly and so that they can be saved in the preset.
    double sliders[3];
    sliders[0] = gainDirectPathSlider.getValue();
    sliders[1] = gainEarlySlider.getValue();
    sliders[2] = gainReverbTailSlider.getValue();

    bool lateEnabled = sourceImagesHandler.enableReverbTail;

    sourceImagesHandler.enableReverbTail = true;
    // this will help us with the loop...
    float values[4] = {0.0f, 0.0f, 0.0f, 0.0f}; // one extra because this loop is ugly.
    string names[3] = {"direct", "early", "late"};

    // records each IR with the right name, path, and sourceImagesHandler gain values.
    for (int i = 0; i < 3;){
        values[i] = 1.0f;

        sourceImagesHandler.directPathGain = values[0];
        sourceImagesHandler.earlyGain = values[1];
        sourceImagesHandler.reverbTailGain = values[2];

        // this should pass in
        recordIr(IRPath, names[i]);


        i++;
        values[i-1] = 0.0f;
    }

    // reset the gain values in the sourceImagesHandler
    sourceImagesHandler.directPathGain = sliders[0];
    sourceImagesHandler.earlyGain = sliders[1];
    sourceImagesHandler.reverbTailGain = sliders[2];
    sourceImagesHandler.enableReverbTail = lateEnabled;

    // ensure that the sliders are in the right places
    gainDirectPathSlider.setValue(sliders[0]);
    gainEarlySlider.setValue(sliders[1]);
    gainReverbTailSlider.setValue(sliders[2]);


    presetPath = originalPresetPath;
}

//==============================================================================
// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent() { return new MainContentComponent(); }
