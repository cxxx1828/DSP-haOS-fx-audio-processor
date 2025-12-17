# HAOS FX Audio Processor
## Real-Time Multichannel Audio Processing Module with MP3 Decoder Support

### Project Overview
This project implements a comprehensive digital signal processing module for the HaOS real-time operating system, featuring a six-channel audio processor with configurable signal processing blocks and dual decoder architecture. The system was developed as part of the Advanced Audio Digital Signal Processing course requirements, demonstrating professional audio processing implementation in embedded systems.

### Core Functionality
The FX processor module combines two input channels (CH0 and CH1) into six independent output channels, each with individually configurable processing parameters. The system implements precise gain stages, programmable delay lines, and finite impulse response (FIR) low-pass filters with selectable cutoff frequencies. The module supports both PCM and MP3 audio input formats through an integrated dual-decoder architecture.

### Technical Specifications
- **Processing Channels**: 6 independent output channels
- **Sample Rate**: 48 kHz (configurable via HaOS API)
- **CH0 Processing**: -1.8 dB initial gain, configurable delay (0/150/300/450 ms), -1.2 dB post-delay gain
- **CH1 Processing**: -2.0 dB initial gain, FIR low-pass filter (2/3/4/5 kHz cutoff), -2.2 dB post-filter gain
- **Filter Implementation**: 31-tap FIR filters with optimized circular buffer implementation
- **Memory Architecture**: Pre-allocated delay buffers with maximum 450 ms capacity per channel

### System Integration
The module was successfully integrated into the HaOS real-time operating system following established integration patterns:
- **MCV Structure**: Module Control Variables for parameter management
- **MCT Configuration**: Module Call Table with real-time callback functions
- **ODT Setup**: Object Descriptor Table for system-level integration
- **API Compatibility**: Full compliance with HaOS audio processing pipeline

### MP3 Decoder Extension
A significant project extension involved integrating MP3 decoding capability alongside the existing PCM decoder:
- **Dual Decoder Architecture**: Runtime-selectable decoder chain (PCM or MP3)
- **Application Switching**: Command-line parameter (`--app 0/1`) for decoder selection
- **Format Transparency**: Identical audio processing regardless of input format
- **Test Validation**: Comprehensive testing with FFmpeg-generated MP3 test vectors
- **Bit-Perfect Consistency**: Verified identical processing results between PCM and MP3 input paths

### Development Methodology
The project followed a structured development approach:

1. **Standalone Implementation**: Initial development as a command-line WAV processor with complete parameter configurability
2. **HaOS Integration**: Adaptation of processing algorithms to HaOS real-time constraints and API requirements
3. **MP3 Extension**: Integration of MP3 decoder using libmp3lame library with HaOS module specifications
4. **Validation Framework**: Implementation of automated testing using GRANT/CUTE systems

### Testing and Validation
A comprehensive testing strategy was employed:
- **Unit Testing**: Individual block validation using CUTE framework
- **Integration Testing**: End-to-end validation using GRANT automation
- **Bit-Perfect Verification**: Wave comparison between standalone and HaOS implementations
- **Performance Metrics**: CPU utilization measurement and memory footprint analysis
- **Subjective Evaluation**: Audacity-based visual analysis of processing effects

### Results and Performance
The implemented system achieved:
- **Processing Accuracy**: Bit-perfect matching between reference and HaOS implementations
- **Real-Time Performance**: Less than 5% CPU utilization at 48 kHz with six active channels
- **Memory Efficiency**: Approximately 1 MB total memory footprint
- **Latency Control**: Precise delay implementation with sample-level accuracy
- **Test Coverage**: 100% validation of all processing paths and configuration combinations



### Author
Nina Dragićević 
