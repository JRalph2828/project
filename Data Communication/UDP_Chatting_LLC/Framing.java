package UDP_Chatting_LLC;

public class Framing {

	public static byte[] outdata = new byte[518];
	public CRC c = new CRC();

	// I-format
	public Framing(byte[] DST_ADDR, byte[] SRC_ADDR, byte[] LEN_PDU, byte DSAP, byte SSAP, byte[] CONTROL,
			byte[] DATA) {
		
		System.arraycopy(DST_ADDR, 0, outdata, 0, 6);
		System.arraycopy(SRC_ADDR, 0, outdata, 6, 6);
		System.arraycopy(LEN_PDU, 0, outdata, 12, 2);
		outdata[14] = DSAP;
		outdata[15] = SSAP;
		System.arraycopy(CONTROL, 0, outdata, 16, 2);
		System.arraycopy(DATA, 0, outdata, 18, DATA.length);
		byte[] cache = new byte[18+DATA.length];
		System.arraycopy(outdata, 0, cache, 0, 18+DATA.length);
		byte[] crc = c.CRCmake(cache);
		System.arraycopy(crc, 0, outdata, 18 + DATA.length, 4);
	}

	// S-format
	public Framing(byte[] DST_ADDR, byte[] SRC_ADDR, byte[] LEN_PDU, byte DSAP, byte SSAP, byte[] CONTROL) {
		
		System.arraycopy(DST_ADDR, 0, outdata, 0, 6);
		System.arraycopy(SRC_ADDR, 0, outdata, 6, 6);
		System.arraycopy(LEN_PDU, 0, outdata, 12, 2);
		outdata[14] = DSAP;
		outdata[15] = SSAP;
		System.arraycopy(CONTROL, 0, outdata, 16, 2);
		byte[] cache = new byte[18];
		System.arraycopy(outdata, 0, cache, 0, 18);
		byte[] crc = c.CRCmake(cache);
		System.arraycopy(crc, 0, outdata, 18 , 4);
		}
	
	// U-format
		public Framing(byte[] DST_ADDR, byte[] SRC_ADDR, byte[] LEN_PDU, byte DSAP, byte SSAP, byte CONTROL) {
			
			System.arraycopy(DST_ADDR, 0, outdata, 0, 6);
			System.arraycopy(SRC_ADDR, 0, outdata, 6, 6);
			System.arraycopy(LEN_PDU, 0, outdata, 12, 2);
			outdata[14] = DSAP;
			outdata[15] = SSAP;
			outdata[16] = CONTROL;
			byte[] cache = new byte[17];
			System.arraycopy(outdata, 0, cache, 0, 17);
			byte[] crc = c.CRCmake(cache);
			System.arraycopy(crc, 0, outdata, 17 , 4);
			}

	public byte[] framing() {
		return outdata;
	}

}
