# Sacd
Robert Tari <robert.tari@gmail.com> 
https://tari.in/www/software/sacd/

## Description
Super Audio CD decoder. 
Converts SACD image files, Philips DSDIFF and Sony DSF files to 24-bit high resolution wave files. Handles both DST and DSD streams.

## Usage

sacd -i infile [-o outdir] [options]

  -i, --infile         : Specify the input file (*.iso, *.dsf, *.dff)  
  -o, --outdir         : The folder to write the WAVE files to. If you omit
                         this, the files will be placed in the input file's
                         directory  
  -c, --stdout         : Stdout output (for pipe), sample:
                         sacd -i file.dsf -c | play -  
  -r, --rate           : The output samplerate.
                         Valid rates are: 88200, 96000, 176400 and 192000.
                         If you omit this, 96KHz will be used.  
  -s, --stereo         : Only extract the 2-channel area if it exists.
                         If you omit this, the multichannel area will have priority.  
  -p, --progress       : Display progress to new lines. Use this if you intend
                         to parse the output through a script. This option only
                         lists either one progress percentage per line, or one
                         status/error message.  
  -h, --help           : Show this help message  

