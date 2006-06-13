package jmpg.audio;

import java.io.*;

/* That's a hack 
 *
 * I first tried the same with connected Pipes but this
 * was incredible slow (dunno exact why .. it was just ugly),
 * so I written a simple hack to have a fast pipe-like InputStream
 * 
 * It can also work as a play buffer
 */


public class ContinousByteArrayInputStream extends InputStream {
  int front=0,tail=0;
  int buflen = 65536;
  /* Rinbuffer */
  byte buffer[];
  boolean closed = false;

  ContinousByteArrayInputStream() {
    this(65536);
  }
  ContinousByteArrayInputStream(int size) {
    buflen = size;
    buffer = new byte[buflen];
  }

  private int len() {
    if(front >= tail)
      return front-tail;
    else
      return buflen-(tail - front);
  }

  public int available() {
      return len();
  }
  public void close() {
    closed = true;
  }
  public boolean markSupported() {
    return false;
  }

  public synchronized void addBuffer(byte b[],int offset,int len) throws IOException {
    if(len >= buflen) {
      throw new IOException();
    }
    while( len() + len >= buflen) {
      try {
        wait(); 
      }
      catch(InterruptedException e) { }
    }

    if(buflen - front >= len) {
      System.arraycopy(b,offset,buffer,front,len);
    }
    else {
      int len2 = buflen-front;
      System.arraycopy(b,offset,buffer,front,len2);
      System.arraycopy(b,offset+len2,buffer,0,len-len2);
    }
    front += len;
    if(front >= buflen)
      front -= buflen;
    notify();
  }

  public synchronized int read() throws IOException {
    byte b[] = new byte[1];
    read(b,0,1);
    if(b[0] < 0)
      return (int) b[0] + 256;
    else
      return (int) b[0];
  }
  public synchronized int read(byte b[]) throws IOException {
    return read(b,0,b.length);
  }
  public synchronized int read(byte b[],int offset,int len) throws IOException {
    int len1;
    while( (len1=len()) == 0 ) {
      try {
        wait();
      }
      catch( InterruptedException e) { }
    }
    if(len1 > len)
      len1 = len;
    if(buflen - tail >= len1) {
      System.arraycopy(buffer,tail,b,offset,len1);
    }
    else {
      int len2 = buflen-tail;
      System.arraycopy(buffer,tail,b,offset,len2);
      System.arraycopy(buffer,0,b,offset+len2,len1-len2);
    }
    tail += len1;
    if(tail >= buflen)
      tail -= buflen;
    notify();
    return len1;
  }
}

