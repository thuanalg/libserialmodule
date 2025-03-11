mv trigger_serial.txt trigger_serial.txt_
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH
./test_main --is_baudrate=9600 --is_cfg=simplelog.cfg --is_port=/dev/ttyUSB0
