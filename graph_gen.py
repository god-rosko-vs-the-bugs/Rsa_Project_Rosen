import csv
import pandas
import sys
import os 

def read_csv(name):
    data  = [] 
    with open(name, newline='') as f:
        reader = csv.reader(f)
        data = list(reader)
        data.pop(0)
    print("data",data)
    return data


def gen_graph(listo):
    tmp_list = []
    print(tmp_list)
    print ("appending ", listo[0])
    tmp_list.append(read_csv(listo[0]))
    print(tmp_list)

def get_names_of_files(name_wild, dir_thr):
    path = './res/'+ str(dir_thr) + '/'
    files = []
    print(name_wild)
    for r, d, f in os.walk(path):
        for file in f:
            if name_wild in file:
                files.append(os.path.join(r, file))
    return files


sys.argv.pop(0) 
print("fname = ",sys.argv[0])
#for i in range(16):
res = get_names_of_files('bench-results-' + sys.argv[0] + '-',1 )
res.sort()
gen_graph(res)
