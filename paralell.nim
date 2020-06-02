import locks,os,strutils,tables,sequtils


proc read_file(f:File,start_pos,str_size:int64,read:var int64):CountTable[char]=
    var p = newString(str_size)
    if readChars(f,p,start_pos,str_size) > 0:
        return p.toCountTable() 
    else:
        return "".toCountTable()

proc Partition(max:int64,threads:Natural): seq[int64]=
    var load:int64 = max div threads
    if load > 0:
        var res = toSeq(0..(threads-1)).mapIt(it * load )
        res[threads-1] = res[threads-1] + max mod threads    
        return res

var f = open(paramStr(1))
var f_size:int64 = getFileSize(f)
var p = newString(f_size)
var thr_c:int = 4




var read = readChars(f,p,0,f_size)
echo ("read :",read)
var res= read_file(p) 
echo res   
close(f)
