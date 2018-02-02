package UDP_Chatting_LLC;

import java.util.zip.CRC32;
import java.nio.*;

public class CRC {

	CRC32 crc = new CRC32();

	// byte[] ==> CRC
	public byte[] CRCmake(byte[] data) {
		crc.update(data, 0, data.length);
		long temp = crc.getValue();
		byte[] buff = new byte[4];
		buff[3] = (byte) (int) (temp & 255L);
		buff[2] = (byte) (int) (temp >>> 8 & 255L);
		buff[1] = (byte) (int) (temp >>> 16 & 255L);
		buff[0] = (byte) (int) (temp >>> 24 & 255L);
		return buff;
	}

	// compare 2 CRC
	public boolean isequalCRC(byte[] crc1, byte[] crc2) {
		long num1, num2;
		num1 = bytesToLong(crc1);
		num2 = bytesToLong(crc2);
		if (num1 == num2)
			return true;
		else
			return false;
	}

	// byte[] ==> long
	public static long bytesToLong(byte[] bytes) {
		ByteBuffer buffer = ByteBuffer.allocate(1024);
		byte[] ped = { 0x00, 0x00, 0x00, 0x00 };
		buffer.put(ped);
		buffer.put(bytes);
		buffer.flip();
		return buffer.getLong();
	}
}
