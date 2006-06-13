
package jmpg.audio;

import java.lang.*;
import java.io.*;
import sun.audio.*;

public class SunAudio extends OutputStream implements Audio,Runnable
{ 
  ContinousByteArrayInputStream cin;

  byte head[]   = { 0x2e,0x73,0x6e,0x64 , 0x00,0x00,0x00,0x20 ,
		    -1,-1,-1,-1 , 0x00,0x00,0x00,0x03 ,
                    0x00,0x00,-0x54,0x44 , 0x00,0x00,0x00,0x02 ,
 		    0x00,0x00,0x00,0x00 , 0x00,0x00,0x00,0x00 };
  int init = 0;
  
  public SunAudio() throws IOException {
     cin = new ContinousByteArrayInputStream();
     cin.addBuffer(head,0,32);
  }

  public SunAudio(int bufsize) throws IOException {
     cin = new ContinousByteArrayInputStream(bufsize);
     cin.addBuffer(head,0,32);
  }

  public void write(byte b[]) throws IOException {
System.out.println("D5");
    write(b,0,b.length);
  }

  public void write(int b) throws IOException {
System.out.println("D6");
    byte data[] = new byte[1];
    data[0] = (byte) b;
    write(data);
  }

  public void write(byte b[],int offset,int len) throws IOException {
    cin.addBuffer(b,offset,len);
    if(init == 0) {
    new Thread(this).start();
      init = 1;
    }
  }

/*
  public void write(byte b[],int offset,int len) throws IOException {
    if(buflen == 0) {
       System.arraycopy(head,0,buffer,0,32);
       buflen = 32;
    }
    System.arraycopy(b,offset,buffer,buflen,len);
    buflen += len;
    if(buflen > 512000) {
      ByteArrayInputStream is = new ByteArrayInputStream(buffer,0,buflen);
      AudioPlayer.player.start(is);
      bufno += 1;
      bufno = bufno & 1;
      buffer = pbuff[bufno];
      buflen = 0;
try {
Thread.sleep(3000);
} catch(Exception e) { }
    }
  }
*/
 
  public int play(byte b[],int offset,int len) throws IOException {
    write(b,offset,len);
    return len;
  }

  public void run() {
    AudioPlayer.player.start(cin);
  }
}

