mv trigger_serial.txt trigger_serial.txt_
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH:/usr/local/lib
./test_main --is_baudrate=115200 --is_cfg=simplelog.cfg --is_port=/dev/ttyUSB0,/dev/ttyUSB1 --is_offdsr=1
