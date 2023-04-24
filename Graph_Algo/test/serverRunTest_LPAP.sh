cd ../build/bin
nohup ./srv_UtilServerTest_LabelPropagationGPU 23947348 6661515 4 0 > srv1.txt 2>&1 &
nohup ./srv_UtilServerTest_LabelPropagationGPU 23947348 6660790 4 1 > srv2.txt 2>&1 &
nohup ./srv_UtilServerTest_LabelPropagationGPU 23947348 6661058 4 2 > srv3.txt 2>&1 &
nohup ./srv_UtilServerTest_LabelPropagationGPU 23947348 6661543 4 3 > srv4.txt 2>&1 &
