
package jmpg.audio;

import java.io.*;

public interface Audio {
  abstract int play(byte b[],int offset,int len) throws IOException;
}

