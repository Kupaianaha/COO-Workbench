/**
 * This file serves as a development workbench for camera logic.
 * The functions defined here are intended for integration into 
 * the larger camera-interface project.
 * * Included functions:
 * - fexpose(): Handles camera exposure timing and logic.
 * - fabort(): Safety function to terminate current operations.
 * - fread_frame(): Captures and reads a single frame buffer.
 * - roi(): Sets the Region of Interest parameters.
 * - reset_roi(): Restores the sensor to full-frame readout.
 */

#include <vector>   // for timing stats vectors
#include <chrono>   // for taking time stamps
#include <ctime>    // for time_t, tm, localtime, etc.


    this->pause_log = false;
    this->abort_fexpose = false;
    this->fread = 0; 

/**************** Archon::Interface::reset_roi ****************************/
/**
    * @fn     reset_roi
    * @brief  reset roi limits for h2rg to full frame
    * @param  NONE
    * @return ERROR or NO_ERROR
    *
    * NOTE: This could possibly be achived by calling the default mode 
            and just resetting the taplines here
    */
long Interface::reset_roi() {
    std::string function = "Archon::Interface::reset_roi";
    std::stringstream message;
    std::stringstream cmd;
    std::string return_str;
    long error = NO_ERROR;

    // Set Parameters for ROI here
    //H2RG_ rows, window_rows, columns, window_columns, rows_skip
    int cols = 512;
    int rows = 2048;
    int skip = 0;
    cmd << "H2RG_win_columns "<< cols;
    error = this->set_parameter(cmd.str());
    cmd.str("");
    cmd << "H2RG_win_rows "<< rows;
    if (error == NO_ERROR) error = this->set_parameter(cmd.str());
    cmd.str("");
    cmd << "H2RG_columns "<< cols;
    if (error == NO_ERROR) error = this->set_parameter(cmd.str());
    cmd.str("");
    cmd << "H2RG_rows "<< rows;
    if (error == NO_ERROR) error = this->set_parameter(cmd.str());
    cmd.str("");
    cmd << "H2RG_rows_skip "<< skip;
    if (error == NO_ERROR) error = this->set_parameter(cmd.str());
    if (error != NO_ERROR) {
    message.str(""); message << "***ERROR setting roi parameters***" << cmd.str();
    this->camera.log_error( function, message.str() );
    return ERROR;
    }

    // Configure CDS
    //pixelpertap/pixelcount, linespertap/linecount
    error = this->cds("PIXELCOUNT 512", return_str);
    if (error == NO_ERROR) error = this->cds("LINECOUNT 2048", return_str);
    if (error != NO_ERROR) {
    message.str(""); message << "***ERROR setting CDS parameters***" << cmd.str();
    this->camera.log_error( function, message.str() );
    return ERROR;
    }

    // Configure taplines (cds)
    //5 taplines original config
    //used store values to reset
    std::string taplines_str = (this->taplines_store == 0)? "5": std::to_string(this->taplines_store); // if store is empty then use 5 as default
    error = this->cds("TAPLINES " + taplines_str, return_str);
    if (error == NO_ERROR) error = this->cds("TAPLINE0 " + this->tapline0_store, return_str);
    if (error == NO_ERROR) error = this->cds("TAPLINE1 " + this->tapline1_store, return_str);
    if (error == NO_ERROR) error = this->cds("TAPLINE2 " + this->tapline2_store, return_str);
    if (error == NO_ERROR) error = this->cds("TAPLINE3 " + this->tapline3_store, return_str);
    if (error == NO_ERROR) error = this->cds("TAPLINE4 " + this->tapline4_store, return_str);
    if (error != NO_ERROR) {
    message.str(""); message << "***ERROR setting taplines***" << taplines_str;
    this->camera.log_error( function, message.str() );
    }
    return (error);
}
/**************** Archon::Interface::reset_roi ****************************/

/**************** Archon::Interface:: roi *********************************/
/**
    * @fn     roi
    * @brief  set roi limits for h2rg
    * @param  geom_in  string, with vstart vstop hstart hstop in pixels
    * @return ERROR or NO_ERROR
    *
    * NOTE:
    */
long Interface::roi(std::string geom_in, std::string &retstring) {
    std::string function = "Archon::Interface::roi";
    std::stringstream message;
    std::stringstream cmd;
    std::string dontcare;
    int hstart, hstop, vstart, vstop;
    long error = NO_ERROR;
    std::vector<std::string> tokens;

    //Set Geometry of the roi here for later reference
    if ( !geom_in.empty() ) {         // geometry arguments passed in
    Tokenize(geom_in, tokens, " ");
    if (tokens.size() != 4) {
        message.str(""); message << "param expected 4 arguments (vstart, vstop, hstart, hstop) but got " << tokens.size();
        this->camera.log_error( function, message.str() );
        return ERROR;
    }
    try {
        vstart = std::stoi( tokens[0] ); // test that inputs are integers
        vstop = std::stoi( tokens[1] );
        hstart = std::stoi( tokens[2] );
        hstop = std::stoi( tokens[3]);

    } catch (std::invalid_argument &) {
        message.str(""); message << "unable to convert geometry values: " << geom_in << " to integer";
        this->camera.log_error( function, message.str() );
        return ERROR;

    } catch (std::out_of_range &) {
        message.str(""); message << "geometry values " << geom_in << " outside integer range";
        this->camera.log_error( function, message.str() );
        return ERROR;
    }

    // Validate values are within detector
    if ( vstart < 0 || vstop > 2047 || hstart < 0 || hstop > 2047) {
        message.str(""); message << "geometry values " << geom_in << " outside pixel range";
        this->camera.log_error( function, message.str());
        return ERROR;
    }
    // Validate values have proper ordering
    if (vstart >= vstop || hstart >= hstop) {
        message.str(""); message << "geometry values " << geom_in << " are not correctly ordered";
        this->camera.log_error( function, message.str());
        return ERROR;
    }

    // Set Parameters for ROI here
        //H2RG_ rows, window_rows, columns, window_columns, rows_skip
    this->win_vstart = vstart; // set y lo lim
    this->win_vstop = vstop; // set y hi lim
    this->win_hstart = hstart; // set x lo lim
    this->win_hstop = hstop; // set roi x hi lim
    int rows = (this->win_vstop - this->win_vstart) + 1;
    int cols = (this->win_hstop - this->win_hstart) + 1;
    cmd.str("");
    if (error == NO_ERROR) {
        cmd << "H2RG_win_columns " << cols;
        error = this->set_parameter(cmd.str());
    }
    cmd.str("");
    if (error == NO_ERROR) {
        cmd << "H2RG_win_rows " << rows;
        error = this->set_parameter(cmd.str());
    }
    cmd.str("");
    if (error == NO_ERROR) {
        cmd << "H2RG_columns " << cols;
        error = this->set_parameter(cmd.str());
    }
    cmd.str("");
    if (error == NO_ERROR) {
        cmd << "H2RG_rows " << rows;
        error = this->set_parameter(cmd.str());
    }
    if (error == NO_ERROR) {
        cmd.str("");
        cmd << "H2RG_rows_skip " << this->win_vstart;
        error = this->set_parameter(cmd.str());
    }

    // set cds values
        //pixelpertap/pixelcount, linespertap/linecount
        //Pixelcount changes based on mode, rx, rxr, utr
        //TODO:: set pixelcount based on mode
    int pixelcount = cols / 2;
    cmd.str("");
    cmd << "PIXELCOUNT " << pixelcount;
    error = this->cds(cmd.str(), dontcare);
    cmd.str("");
    cmd << "LINECOUNT " << rows;
    error = this->cds(cmd.str(), dontcare);

    //configure taplines (cds)
        //2 taplines, booth taplines currounding the center of the detector
        //Store old tapline values for later reference
    std::string taplines_str;
    this->cds("TAPLINES", taplines_str);
    this->taplines_store = std::stoi(taplines_str);

    std::string tapline0;
    this->cds("TAPLINE0", tapline0);
    this->tapline0_store = tapline0;
    std::string tapline1;
    this->cds("TAPLINE1", tapline1);
    this->tapline1_store = tapline1;
    std::string tapline2;
    this->cds("TAPLINE2", tapline2);
    this->tapline2_store = tapline2;
    std::string tapline3;
    this->cds("TAPLINE3", tapline3);
    this->tapline3_store = tapline3;
    std::string tapline4;
    this->cds("TAPLINE4", tapline4);
    this->tapline4_store = tapline4;

    if (error == NO_ERROR) {
        error = this->cds("TAPLINES 2", dontcare);
    }
    this->taplines = 2;
    if (error == NO_ERROR) {
        error = this->cds("TAPLINE0 AM29L,1,0", dontcare);
    }
    if (error == NO_ERROR) {
        error = this->cds("TAPLINE1 AM25R,1,0", dontcare);
    }

    // update modemap, in case someone asks again
    std::string mode = this->camera_info.current_observing_mode;

    this->modemap[mode].geometry.linecount = rows;
    this->modemap[mode].geometry.pixelcount = cols;
    this->camera_info.region_of_interest[0] = this->win_hstart;
    this->camera_info.region_of_interest[1] = this->win_hstop;
    this->camera_info.region_of_interest[2] = this->win_vstart;
    this->camera_info.region_of_interest[3] = this->win_vstop;
    this->camera_info.detector_pixels[0] = cols;
    this->camera_info.detector_pixels[1] = rows;

    this->camera_info.set_axes();

    //Resize Image data size based on new geometry
    int num_detect = this->modemap[mode].geometry.num_detect;
    this->image_data_bytes = (uint32_t) floor( ((this->camera_info.image_memory * num_detect) + BLOCK_LEN - 1 ) / BLOCK_LEN ) * BLOCK_LEN;

    if (this->image_data_bytes == 0) {
        this->camera.log_error( function, "image data size is zero! check NUM_DETECT, HORI_AMPS, VERT_AMPS in .acf file" );
        error = ERROR;
    }
    } else {
    //print region of interest and geometry info for current mode
    std::string mode = this->camera_info.current_observing_mode;

    std::ostringstream ss;

    ss << "===== Camera Geometry Update =====\n"
        << "Mode: " << mode << "\n"
        << "Geometry -> "
        << "Lines: " << this->modemap[mode].geometry.linecount
        << ", Pixels: " << this->modemap[mode].geometry.pixelcount
        << ", Num Detect: " << this->modemap[mode].geometry.num_detect << "\n"
        << "ROI -> ["
        << this->camera_info.region_of_interest[0] << ", "
        << this->camera_info.region_of_interest[1] << ", "
        << this->camera_info.region_of_interest[2] << ", "
        << this->camera_info.region_of_interest[3] << "]\n"
        << "Detector Pixels -> Cols: "
        << this->camera_info.detector_pixels[0]
        << ", Rows: "
        << this->camera_info.detector_pixels[1] << "\n"
        << "Image Memory: " << this->camera_info.image_memory << "\n"
        << "Image Data Bytes (block aligned): " << this->image_data_bytes << "\n"
        << "==================================\n";

    std::string debug_string = ss.str();
    logwrite( function, debug_string );
    error = NO_ERROR;
    }
    return (error);
}
/**************** Archon::Interface:: roi *********************************/

/**************** Archon::Interface::fexpose ******************************/
/**
    * @fn     fexpose
    * @brief  initiate continuous for h2rg
    * @param  nseq_in string, Should match a valid const to trigger continuous mode
    * @return ERROR or NO_ERROR
    *
    * This function does the following before returning successful completion:
    *  1) trigger an Archon exposure by setting the EXPOSE parameter = nseq_in
    *  2) wait for exposure delay
    *  3) wait for readout into Archon frame buffer
    *  4) read frame buffer from Archon to host
    *  5) Do NOT write frame to disk (eventually to shared memory)
    *
    * Note that this assumes that the Archon ACF has been programmed to automatically
    * read out the detector into the frame buffer after an exposure.
    *
    */
long Interface::fexpose(int nseq_in) {
    //Iniciate variables and messages for logs
    const std::string function = "Archon::Interface::fexpose";
    std::stringstream message;
    long error = NO_ERROR;
    constexpr bool is_debug = false;
    //bool is_timing_test = true;
    this->abort_fexpose = false;
    this->get_frame_status(); // initial frame status read
    this->lastframe = this->frame.bufframen[this->frame.index];
    this->pause_log = true;  // pause logging to avoid log overflow during continuous exposures
    int startframe = this->lastframe;
    //int sample_size = 1000;
    //double results[sample_size][6] = {};
    double frequency = 50.0; // target frequency in Hz for exposures
    //std::time_t now = std::time(nullptr);
    //std::tm* tm_ptr = std::localtime(&now);
    //std::ostringstream oss;
    //oss << std::put_time(tm_ptr, "%Y%m%d_%H%M%S");
    //std::string save_filename = "/home/hsdev/camera-dev/freedom/timing_results/fexpose_results_" + oss.str() + ".csv";

    if (!this->modeselected) {
        this->camera.log_error(function, "no mode selected");
        return ERROR;
    }

    if (this->exposeparam.empty()) {
        message << "EXPOSE_PARAM not defined in configuration file " << this->config.filename;
        this->camera.log_error(function, message.str());
        return ERROR;
    }

    if (this->is_freerun == false) {
        message << "is_freerun mode not activated" << this->config.filename;
        this->camera.log_error(function, message.str());
        return ERROR;
    }

    //Send Expose command to Archon to start freerun exposures
    error = this->prep_parameter(this->exposeparam, std::to_string(1));
    if (error == NO_ERROR) {
        error = this->load_parameter(this->exposeparam, std::to_string(1));
    }
    if (error != NO_ERROR) {
        logwrite(function, "ERROR: Failed to initiate continuous exposure.");
        return error;
    }


    // Put some debugging logic in for checks
    if (is_debug) {
    logwrite(function, "Starting continuous exposure...");
    }

    //time stamps, starting point and needed variables
    //if (is_timing_test) {
    //  logwrite(function, "Starting timing test for fexpose...");
    //}
    auto period = std::chrono::duration<double, std::milli>(1000.0 / frequency);// turn into microseconds!
    auto Signal_t0 = std::chrono::high_resolution_clock::now(); 
    //int loopIter = -1;

    //Start endless readout loop while(true)
    while (this->abort_fexpose == false && this->is_freerun == true && this->abort == false) {
    //Timing test: capture timestamp at start of loop
    //loopIter++;
    // Checking for frame status to iterate on the next frame
    //if (is_timing_test && (loopIter <= sample_size)) {
    //  results[loopIter][0] = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-Signal_t0).count();
    //}
    while (true) {
        error = this->get_frame_status();
        if (error != NO_ERROR) {
        logwrite(function, "ERROR: getting frame status");
        return error;
        }
        if (this->frame.bufframen[this->frame.index] != this->lastframe) {
        this->lastframe = this->frame.bufframen[this->frame.index];
        break;
        }
        if (this->abort_fexpose || this->abort) {
        break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500)); // sleep briefly to avoid busy waiting
    }
    //if (is_timing_test && (loopIter <= sample_size)) {
    //  results[loopIter][1] = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-Signal_t0).count();
    //}
    if ( error != NO_ERROR ) {
        logwrite( function, "ERROR: waiting for readout" );
        return error;
    }
    //gather frame and get error if occured
    if (!this->abort_fexpose && !this->abort) {
        error = fread_frame();
    }
    //if (is_timing_test && (loopIter <= sample_size)) {
    //  results[loopIter][2] = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-Signal_t0).count();
    //}
    if (error != NO_ERROR) {
        logwrite(function, "ERROR: reading frame buffer");
        return error;
    }

    if (is_debug) {
        if (error == NO_ERROR) {
        printf("Frame acquired\n");
        }
    }
    //shared memory writing timing
    //if (is_timing_test && (loopIter <= sample_size)) {
    //  results[loopIter][4] = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-Signal_t0).count();
    //}
    // Wait for correct time for frame to get pushed into shm
    while((std::chrono::high_resolution_clock::now()-Signal_t0) < period)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }

    //Write to Shared Memory

    //if (is_timing_test && (loopIter <= sample_size)) {
    //  results[loopIter][5] = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()-Signal_t0).count();
    //}

    //Take Timing for next loop
    Signal_t0 = std::chrono::high_resolution_clock::now();

    if (this->abort_fexpose || this->abort) {
        logwrite(function, "Aborting... ");
    }

    //if (is_timing_test) {
    //  results[loopIter][3] = 0.0; // placeholder for shared memory write timing, to be filled in when that is implemented
    //}
    //if (is_timing_test && (loopIter == sample_size)) {
    //  this->writeSignalCSV(results, save_filename, sample_size);
    //}
    }
    printf("fexpose loop end.\n");
    // --- Sequence Complete ---
    error = get_frame_status();  // get final frame status
    int lastframe = this->frame.bufframen[this->frame.index];
    this->fread = lastframe - startframe;

    this->pause_log = false;  // unpause logging now that we're done with continuous exposures
    return error;
}
/**************** Archon::Interface::fexpose *********************************/

/**************** Archon::Interface::fabort ******************************/
/**
    * @fn     fabort
    * @brief  aborts freedom exposure for h2rg
    * @param  retstring string
    * @return ERROR or NO_ERROR
    *
    * This function does the following before returning successful completion:
    *  1) aborts the continuous exposure started by fexpose
    *  2) set freerun mode/flags off (0 = off)
    *
    */
long Interface::fabort(std::string &retstring) {
    //Iniciate variables and messages for logs
    const std::string function = "Archon::Interface::fabort";
    std::stringstream message;
    long error = NO_ERROR;
    int nread = this->fread;  // number of frames read during fexpose

    //Set abort flag to true to exit fexpose loop
    this->abort_fexpose = true;

    //Sleep for a short time to allow fexpose to exit
    //fexpose_thread.join();
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    //Set freerun mode off
    error = this->set_freerun("false");
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // give it a moment to take effect
    error = this->set_freerun("false"); // call it twice to ensure it takes effect
    if (error != NO_ERROR) {
        message << "ERROR: setting freerun mode off";
        this->camera.log_error(function, message.str());
        retstring = message.str();
        return ERROR;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // give it a moment to take effect
    retstring = "Freerun exposure aborted successfully";
    std::cout << nread << " frames read" << std::endl;
    std::cout << std::fixed << std::setprecision(4) << "Median Loop Time = " << fetch_stats.median() << " us"
            << " | Mean Loop Time = " << fetch_stats.mean() << " us"
            << " | Jitter = " << fetch_stats.jitter() << " us" << std::endl;

    std::cout << "Expose Hz = " << fetch_stats.hertz() << " +- " << fetch_stats.hertz_stddev() << std::endl;
    std::cout << std::fixed << std::setprecision(4) << "Archon time deltas median: " << archon_ts_deltas.median()
        << " us | Mean time " << archon_ts_deltas.mean()
        << " us | Jitter = " << archon_ts_deltas.jitter() << " us" << std::endl;

    // Create a filename with date-time
    //std::ostringstream filename;
    //auto now = std::chrono::system_clock::now();
    //std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    //filename << "/tmp/fetch_stats_" << std::put_time(std::localtime(&now_time), "%Y%m%d_%H%M%S") << ".csv";

    // Write fetch_stats to file
    //std::ofstream out(filename.str());
    //for (double d : fetch_stats.durations_us) {
    //  out << d << "\n";
    //}
    //out.close();

    message.str("");
    message << "READOUT SEQUENCE " << (error == NO_ERROR ? "COMPLETE" : "ERROR")
            << " (" << nread << " frames read)";
    this->camera.async.enqueue(message.str());
    if (error == NO_ERROR) {
    logwrite(function, message.str());
    } else {
    this->camera.log_error(function, message.str());
    }

    // Final frame info
    logwrite(function, "frame index: " + std::to_string(this->frame.index) +
                    ", frame number: " + std::to_string(this->frame.bufframen[this->frame.index]));

    logwrite(function, "Last frame read " + std::to_string(this->frame.frame) +
                    " from buffer " + std::to_string(this->frame.index + 1));
    return error;
}
/**************** Archon::Interface::fabort *********************************/

/**************** Archon::Interface::fread_frame *****************************/
/**
* @fn     fread_frame
* @brief  read latest Archon frame buffer
* @param  None
* @return ERROR or NO_ERROR
*
* This is the fread_frame function which performs the actual read of the
* selected frame type. used for freerun mode posting to shared memory
*/
long Interface::fread_frame() {
std::string function = "Archon::Interface::fread_frame";
std::stringstream message;
int retval;
int bufready;
char check[5], header[5];
char *ptr_image;
int bytesread, totalbytesread, toread;
uint64_t bufaddr;
unsigned int block, bufblocks=0;
long error = ERROR;
int num_detect = this->modemap[this->camera_info.current_observing_mode].geometry.num_detect;

//this->camera_info.frame_type = frame_type;

static uint64_t prev_archon_ts = 0;

error = this->prepare_image_buffer();
if (error == ERROR) {
    logwrite( function, "ERROR: unable to allocate an image buffer" );
    return ERROR;
}

// Archon buffer number of the last frame read into memory
//
bufready = this->frame.index + 1;

if (bufready < 1 || bufready > this->camera_info.activebufs) {
    message.str(""); message << "invalid Archon buffer " << bufready << " requested. Expected {1:" << this->camera_info.activebufs << "}";
    this->camera.log_error( function, message.str() );
    return ERROR;
}

if (this->pause_log == false) {
    message.str(""); message << "will read " << "image"//(frame_type == Camera::FRAME_RAW ? "raw" : "image")
                            << " data from Archon controller buffer " << bufready << " frame " << this->frame.frame;
    logwrite(function, message.str());
}

// Lock the frame buffer before reading it
//
if ( this->lock_buffer(bufready) == ERROR) {
    logwrite( function, "ERROR locking frame buffer" );
    return (ERROR);
}

// Archon buffer base address
bufaddr   = this->frame.bufbase[this->frame.index];

// Calculate the number of blocks expected. image_memory is bytes per detector
bufblocks =
(unsigned int) floor( ((this->camera_info.image_memory * num_detect) + BLOCK_LEN - 1 ) / BLOCK_LEN );

if (this->pause_log == false) {
    message.str(""); message << "will read " << std::dec << this->camera_info.image_memory << " bytes "
                        << "0x" << std::uppercase << std::hex << bufblocks << " blocks from bufaddr=0x" << bufaddr;
    logwrite(function, message.str());
}

// Dont't send fetch command in autofetch mode
// send the FETCH command.
// This will take the archon_busy semaphore, but not release it -- must release in this function!
//
error = this->fetch(bufaddr, bufblocks);
if ( error != NO_ERROR ) {
    logwrite( function, "ERROR: fetching Archon buffer" );
    return error;
}

// Read the data from the connected socket into memory, one block at a time
//
ptr_image = this->image_data;
totalbytesread = 0;
if (this->pause_log == false) {
    std::cerr << "reading bytes: ";
}
for (block=0; block<bufblocks; block++) {
    // Are there data to read?
    if ( (retval=this->archon.Poll()) <= 0) {
    if (retval==0) {
        message.str("");
        message << "Poll timeout waiting for Archon frame data";
        error = ERROR;
    }  // TODO should error=TIMEOUT?

    if (retval<0)  {
        message.str("");
        message << "Poll error waiting for Archon frame data";
        error = ERROR;
    }

    if ( error != NO_ERROR ) this->camera.log_error( function, message.str() );
    break;                         // breaks out of for loop
    }

    // Wait for a block+header Bytes to be available
    // (but don't wait more than 1 second -- this should be tens of microseconds or less)
    //
    auto start = std::chrono::steady_clock::now();             // start a timer now

    while ( this->archon.Bytes_ready() < (BLOCK_LEN+4) ) {
    auto now = std::chrono::steady_clock::now();             // check the time again
    std::chrono::duration<double> diff = now-start;          // calculate the duration
    if (diff.count() > 1) {                                  // break while loop if duration > 1 second
        std::cerr << "\n";
        this->camera.log_error( function, "timeout waiting for data from Archon" );
        error = ERROR;
        break;                       // breaks out of while loop
    }
    }
    if ( error != NO_ERROR ) break;  // needed to also break out of for loop on error

    // Check message header
    SNPRINTF(check, "<%02X:", this->msgref);

    if ( (retval=this->archon.Read(header, 4)) != 4 ) {
    message.str(""); message << "code " << retval << " reading Archon frame header";
    this->camera.log_error( function, message.str() );
    error = ERROR;
    break;                         // break out of for loop
    }

    if (header[0] == '?') {  // Archon retured an error
    message.str(""); message << "Archon returned \'?\' reading " << "image"//(frame_type == Camera::FRAME_RAW ? "raw" : "image") << 
                                " data";
    this->camera.log_error( function, message.str() );
    this->fetchlog();      // check the Archon log for error messages
    error = ERROR;
    break;                         // break out of for loop

    } else if (strncmp(header, check, 4) != 0) {
    message.str(""); message << "Archon command-reply mismatch reading " << "image"//(frame_type == Camera::FRAME_RAW ? "raw" : "image")
                                << " data. header=" << header << " check=" << check;
    this->camera.log_error( function, message.str() );
    error = ERROR;
    break;                         // break out of for loop
    }

    // Read the frame contents
    //
    bytesread = 0;
    do {
    toread = BLOCK_LEN - bytesread;
    if ( (retval=this->archon.Read(ptr_image, (size_t)toread)) > 0 ) {
        bytesread += retval;         // this will get zeroed after each block
        totalbytesread += retval;    // this won't (used only for info purposes)
        if (this->pause_log == false) {
        std::cerr << std::setw(10) << totalbytesread << "\b\b\b\b\b\b\b\b\b\b";
        }
        ptr_image += retval;         // advance pointer
    }
    } while (bytesread < BLOCK_LEN);

} // end of loop: for (block=0; block<bufblocks; block++)

// Parse header on first block
if (block == 0) {
    std::string frame_header(this->image_data, 36);
    try {
        int frame_number = std::stoi(frame_header.substr(4, 8), nullptr, 16);
        uint64_t timestamp = std::stoull(frame_header.substr(20, 16), nullptr, 16);

        this->frame.frame = frame_number;
        this->frame.bufframen[this->frame.index] = frame_number;


        if (prev_archon_ts != 0) {
            double delta_us = (timestamp - prev_archon_ts) * 0.01;
            archon_ts_deltas.add(delta_us);
        }
        prev_archon_ts = timestamp;

        //if (is_debug) {
        //  logwrite(function, "frame number: " + std::to_string(frame_number));
        //  logwrite(function, "timestamp: " + std::to_string(timestamp));
        //}
    } catch (const std::exception& e) {
        this->camera.log_error(function, "Failed to parse header: " + std::string(e.what()));
        return ERROR;
    }
}

// give back the archon_busy semaphore to allow other threads to access the Archon now
//
const std::unique_lock<std::mutex> lock(this->archon_mutex);
this->archon_busy = false;
this->archon_mutex.unlock();
if (this->pause_log == false){ 
    std::cerr << std::setw(10) << totalbytesread << " complete\n";   // display progress on same line of std err
}
// If we broke out of the for loop for an error then report incomplete read
//
if ( error==ERROR || block < bufblocks) {
    message.str(""); message << "incomplete frame read " << std::dec
                            << totalbytesread << " bytes: " << block << " of " << bufblocks << " 1024-byte blocks";
    logwrite( function, message.str() );
}

// Unlock the frame buffer
//
if (error == NO_ERROR) error = this->archon_cmd(UNLOCK);

// On success, write the value to the log and return
//
if (error == NO_ERROR) {
    if (this->pause_log == false) {
    message.str(""); message << "successfully read " << std::dec << totalbytesread << "image"//(frame_type == Camera::FRAME_RAW ? "raw" : "image")
                                << " bytes (0x" << std::uppercase << std::hex << bufblocks << " blocks) from Archon controller";
    logwrite(function, message.str());
    }
} else {
    // Throw an error for any other errors
    logwrite( function, "ERROR: reading Archon camera data to memory!" );
}
return error;
}
/**************** Archon::Interface::fread_frame *****************************/

/*=====
 * Function to save an array into a CSV file
 ======*/
 void Interface::writeSignalCSV(double array[][6], const std::string& filename, int Nrows) 
 {
     /* This function saves arrays containing the results of a Signal Test (ie. hardcoded 7col) */
     std::string fullpath = filename;
     std::cout << "Saving to: "<<fullpath<< std::endl;
     std::ofstream file(fullpath);
     
     if (!file.is_open()) 
     {
         std::cout << "Failed to open file: " << filename << std::endl;
         return;
     }
     
     // Get starting precision for the output stream
     int prec = file.precision();
 
     // Now print
     for ( int row = 0; row < Nrows; row++ ) 
     {
         // Print the timestamp with the dedicated formating to ensure no truncation
         file.precision(20);     // Set a large enough precision 
         file << array[row][0] << ",";
         file.precision(prec);   // Reset precision
         // Print the remaining columns using the default precision
         for ( int col = 1; col < 11; col++ ) 
         {
             if(6 < col && col < 10)
             {
                 file.precision(20);
                 file << array[row][col];
             }
             else
             {
                 file.precision(prec);
                 file << array[row][col];
             }
             //file << array[row][col];
             if (col != 11 - 1) 
             {
                 file << ",";
             }
         }
         file << "\n";
     }
     
     file.close();
     
     std::cout << "CSV file written successfully: " << filename << std::endl;
 }
