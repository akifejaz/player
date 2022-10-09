package com.example.player

//create class RingBuffer
class RingBuffer
    (private var size: Int) {
    //create private variables
    private var buffer: ByteArray
    // private var writeIndex: Int
    // private var readIndex: Int
    private var firstelem : Int = 0
    private var last : Int = 0
    private var count: Int

    init {
        buffer = ByteArray(size)
        count = 0
        firstelem = 0
        last = 0
    }

    
    fun size(): Int {                        // return number of items currently in the buffer
       var sum = 0
       for (i in buffer.indices) {
           sum++
       }
       return sum
    }

    fun isEmpty(): Boolean {                 // is the buffer empty (size equals zero)?
        return size() == 0
    }

    fun isFull(): Boolean {                  // is the buffer full (size equals capacity)?
        return size() == size
    }

    fun enqueue(x: Byte) {                   // add item x to the end
        buffer[last] = x
        last++
        if (last == size) last = 0
    }

    fun dequeue(): Byte {                    // delete and return item from the front
        val item = buffer[firstelem]
        buffer[firstelem] = 0
        firstelem++
        if (firstelem == size) firstelem = 0
        return item
    }
}
