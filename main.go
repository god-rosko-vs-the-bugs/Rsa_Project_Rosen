package main

import (
	"container/heap"
	"fmt"
	"io"
	"os"
	"strconv"
	"sync"
	"time"
)

type huffmanTree interface {
	Freq() int
}

type huffmanLeaf struct {
	freq  int
	value rune
}

type huffmanNode struct {
	freq        int
	left, right huffmanTree
}

func (self huffmanLeaf) Freq() int {
	return self.freq
}

func (self huffmanNode) Freq() int {
	return self.freq
}

type treeHeap []huffmanTree

func (th treeHeap) Len() int { return len(th) }
func (th treeHeap) Less(i, j int) bool {
	return th[i].Freq() < th[j].Freq()
}
func (th *treeHeap) Push(ele interface{}) {
	*th = append(*th, ele.(huffmanTree))
}
func (th *treeHeap) Pop() (popped interface{}) {
	popped = (*th)[len(*th)-1]
	*th = (*th)[:len(*th)-1]
	return
}
func (th treeHeap) Swap(i, j int) { th[i], th[j] = th[j], th[i] }

func buildTree(symFreqs map[rune]int) huffmanTree {
	var trees treeHeap
	for c, f := range symFreqs {
		trees = append(trees, huffmanLeaf{f, c})
	}
	heap.Init(&trees)
	for trees.Len() > 1 {
		// two trees with least frequency
		a := heap.Pop(&trees).(huffmanTree)
		b := heap.Pop(&trees).(huffmanTree)

		// put into new node and re-insert into queue
		heap.Push(&trees, huffmanNode{a.Freq() + b.Freq(), a, b})
	}
	return heap.Pop(&trees).(huffmanTree)
}

func printCodes(tree huffmanTree, prefix []byte) {
	switch i := tree.(type) {
	case huffmanLeaf:
		// print out symbol, frequency, and code for this
		// leaf (which is just the prefix)
		fmt.Printf("%c\t%d\t%s\n", i.value, i.freq, string(prefix))
	case huffmanNode:
		// traverse left
		prefix = append(prefix, '0')
		printCodes(i.left, prefix)
		prefix = prefix[:len(prefix)-1]

		// traverse right
		prefix = append(prefix, '1')
		printCodes(i.right, prefix)
		prefix = prefix[:len(prefix)-1]
	}
}

const maxThreads int = 64
const locked int64 = 1

var procTime [maxThreads]int64

var batchSize = [10]int64{512, 1024, 2048, 4096, 8192, 12288, 16384, 20480, 24576, 32768}

var thrSyncers [maxThreads]sync.Mutex

func readFromNBytes(name string, start int64, toRead int64, drain chan []int64, thrInd int, batch int64, loud bool) []int64 {
	if loud {
		fmt.Printf("Thread %d started !\n", thrInd)
	}

	var file, err = os.Open(name)
	var sol = make([]int64, 256)
	var buffer = make([]byte, batch)
	var read int64 = 0
	var i, j int64
	var startT int64 = time.Now().UnixNano()
	var readchars int = 0
	if err != nil {
		return nil
	}
	if toRead > batch {
		for i = 0; i+batch < toRead; i += batch {
			read = read + batch
			file.Seek(start+i, 0)
			readchars, err = io.ReadAtLeast(file, buffer, int(batch))
			if err == nil && readchars > 0 {
				for j = 0; j < int64(batch); j++ {
					sol[buffer[j]]++
				}
			} else {
				if loud {
					fmt.Printf("Thread %d aborted !\n", thrInd)
				}
				return nil
			}
		}
	}
	if read < toRead {
		var bufLen = int(toRead % batch)
		file.Seek(start+read, 0)
		readchars, err = io.ReadAtLeast(file, buffer, bufLen)
		if err == nil && readchars > 0 {
			for j = 0; j < int64(bufLen); j++ {
				sol[buffer[j]]++
			}
		} else {
			if loud {
				fmt.Printf("Thread %d aborted !\n", thrInd)
			}
			return nil
		}
	}
	procTime[thrInd] = time.Now().UnixNano() - startT
	thrSyncers[thrInd].Unlock()
	drain <- sol
	return sol
}

func main() {

	var fileToRead string = ""
	var thr int = 1 // to be set by cmdline args
	var i, j int = 0, 0
	var buffer [maxThreads][]int64
	var drain [maxThreads]chan []int64
	var quietMode = true
	var startLoc [maxThreads]int64
	var args = os.Args[1:]
	var methBatch = 0
	var frequencyTable [256]int64
	var toRead [maxThreads]int64
	var fSize int64 = 0
	var frequencyMap = make(map[rune]int)
	var huffPuff huffmanTree
	var printTree = 0
	var thrDone = 0
	for i = 0; i < len(args); i++ {
		if args[i] == "-f" || args[i] == "-file" {
			fileToRead = args[i+1]
			i++
		} else if args[i] == "-q" || args[i] == "-quiet" {
			quietMode = false
		} else if args[i] == "-t" || args[i] == "-threads" || args[i] == "-tasks" {
			thr, _ = strconv.Atoi(args[i+1])
			i++
		} else if args[i] == "-m" || args[i] == "-meth" {
			methBatch, _ = strconv.Atoi(args[i+1])
			i++
		} else if args[i] == "-ptree" {
			printTree = 1
		}

	}

	var fi, err = os.Stat(fileToRead)
	if err != nil {
		if quietMode {
			fmt.Printf(" No such file! Abort!")
		}
		os.Exit(1)
	}

	fSize = fi.Size()
	var loc int64 = fSize / int64(thr)
	for i = 0; i < thr; i++ {
		toRead[i] = loc
		startLoc[i] = int64(i) * loc
	}
	toRead[thr-1] += fSize % int64(thr)

	for i = range drain {
		drain[i] = make(chan []int64)
	}

	for i = 0; i < thr; i++ {
		thrSyncers[i].Lock()
		go readFromNBytes(fileToRead, startLoc[i], toRead[i], drain[i], i, batchSize[methBatch], quietMode)
	}
	i = 0
	for thrDone < thr {
		thrSyncers[i].Lock()
		buffer[i] = <-drain[i]
		thrDone++
		for j = 0; j < 256; j++ {
			frequencyTable[j] += buffer[i][j]
		}
		i = (i + 1) % thr
	}
	if printTree == 1 {
		for i = 0; i < 256; i++ {
			if frequencyTable[i] > 0 {
				frequencyMap[rune(i)] = int(frequencyTable[i])
			}
		}
		huffPuff = buildTree(frequencyMap)
		printCodes(huffPuff, []byte{})
	}
	var sumMean int64 = 0
	for i = 0; i < thr; i++ {
		sumMean = sumMean + procTime[i]
		fmt.Printf("%d,", procTime[i])
	}
	fmt.Printf("%d;\n", sumMean/int64(thr))
	os.Exit(0)
}
