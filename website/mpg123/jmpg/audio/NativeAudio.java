
package jmpg.audio;

import java.lang.*;
import java.io.*;

public class NativeAudio extends OutputStream implements Audio
{ 
  
  private native int hplay(int handler,byte b[],int offset,int len);
  private native int hopen();
  private native void hclose(int handler);

  byte tb[] = new byte[1];
  int handler;

  static {
    System.loadLibrary("nativeaudio");
  } 

  public NativeAudio() {
    handler = hopen();
  }
  protected void finalize() {
    hclose(this.handler);
  }

  public void write(int b) {
    tb[0] = (byte) b;
    hplay(handler,tb,0,1);
  }
  public int play(byte b[],int offset,int len) {
    return hplay(handler,b,offset,len);
  }
  public void write(byte b[],int offset,int len) {
    hplay(handler,b,offset,len);
  }
  public void write(byte b[]) {
    hplay(handler,b,0,tb.length);
  }

}

