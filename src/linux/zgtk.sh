mv trigger_serial.txt trigger_serial.txt_
export LD_LIBRARY_PATH=./:$LD_LIBRARY_PATH:./
./test_with_gtk --is_baudrate=115200 --is_cfg=simplelog.cfg
