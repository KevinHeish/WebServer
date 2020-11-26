# Buffer Structure 

Buffer is used to write and read in HTTP request and response  

![image](https://github.com/KevinHeish/WebServer/blob/main/others/buf.JPG)  

# Prepare zone  
The prepare zone can be reused if writable size isn't large enough for next writing  
If writable size is less than bytes to write, readable data can be moved forward to make space for writing  
Buffer will be resize if it still can't fit the coming data after above operation
  
# Readable zone  
Position for where read op to start  
Readable size is Write_pos - Read_pos  

# Writable zone  
Position for where write op to start  
Writable size is Buffer Max length - Write_pos  


# Reference  
https://blog.csdn.net/KangRoger/article/details/47344523 (Chinese  
https://blog.csdn.net/kangroger/article/details/47170963 (Chinese  
