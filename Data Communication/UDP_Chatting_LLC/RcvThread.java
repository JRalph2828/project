package UDP_Chatting_LLC;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

import javax.xml.bind.DatatypeConverter;

class RcvThread extends Thread {

	public DatagramSocket socket;
	public DatagramPacket rcv_packet;
	Signaling signal;
	boolean temp = true;
	public int cnt = 0;

	public static final byte FRAME_TYPE_I = 0;

	public static final byte FRAME_TYPE_S = 10;
	public static final byte CONTROL_TYPE_RR = 11;
	public static final byte CONTROL_TYPE_RNR = 12;
	public static final byte CONTROL_TYPE_NAK = 13;

	public static final byte FRAME_TYPE_U = 20;
	public static final byte CONTROL_TYPE_UI = 21;
	public static final byte CONTROL_TYPE_SABME = 22;
	public static final byte CONTROL_TYPE_DISC = 23;
	public static final byte CONTROL_TYPE_FRMR = 24;
	public static final byte CONTROL_TYPE_UA = 25;
	public static final byte CONTROL_TYPE_XID = 26;

	public static byte FRAME_TYPE_FLAG = 98;
	public static byte CONTROL_TYPE_FLAG = 99;

	public static byte dsap = 0;
	public static byte ssap = 0;

	public static byte[] DST_ADDR = new byte[6];
	public static byte[] SRC_ADDR = new byte[6];
	public static byte[] SRC_ADDR_STORE = new byte[6];
	public static byte[] LEN_PDU = new byte[2];
	public static byte DSAP = 0;
	public static byte SSAP = 0;
	public static byte[] CONTROL = new byte[2];
	public static byte[] DATA = new byte[496];
	public static byte[] CRC = new byte[4];

	public static byte NS = 0, NR = 0;
	public static byte seq = 0, ack = 0;
	public static byte[] rcv_msg = new byte[518];

	public static int totallength = 0;
	public static int datalength = 0;

	public static byte[] getLocalMacAddr() {
		// get local IP
		InetAddress ip = null;
		byte[] mac = new byte[6];
		try {
			ip = InetAddress.getLocalHost();
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		// get network interface
		NetworkInterface netif = null;
		try {
			netif = NetworkInterface.getByInetAddress(ip);
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		// if network interface is not NULL
		if (netif != null) {
			// get mac address
			try {
				mac = netif.getHardwareAddress();
			} catch (SocketException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		return mac;
	}

	public String byteArrayToHex(byte[] a) {
		StringBuilder sb = new StringBuilder();
		for (final byte b : a)
			sb.append(String.format("%02xx", b & 0xff));
		return sb.toString();
	}

	public static byte[] longToBytes(long x) {
		ByteBuffer buffer = ByteBuffer.allocate(Long.BYTES);
		buffer.putLong(x);
		return buffer.array();
	}

	public static long bytesToLong(byte[] bytes) {
		ByteBuffer buffer = ByteBuffer.allocate(1024);
		byte[] ped = { 0x00, 0x00, 0x00, 0x00 };
		buffer.put(ped);
		buffer.put(bytes);
		buffer.flip();
		return buffer.getLong();
	}

	RcvThread(DatagramSocket socket, Signaling signal) {
		this.socket = socket;
		this.signal = signal;
	}

	public void run() {

		byte buff[] = new byte[518];
		rcv_packet = new DatagramPacket(buff, buff.length);
		
		SRC_ADDR_STORE = getLocalMacAddr();
		while (temp) {
			try {
				socket.receive(rcv_packet);
				UDPChatting.remoteport = rcv_packet.getPort();
				UDPChatting.remoteaddr = rcv_packet.getAddress();
			} catch (IOException e) {
				System.out.println("Thread exception " + e);
			}

			rcv_msg = rcv_packet.getData();
			/*
			// check for received msg
		    for(int i =0; i<518; i++){
		    	System.out.printf("[%02X]",rcv_msg[i]);
		    }
			System.out.println();
			*/
			
			SRC_ADDR = SRC_ADDR_STORE;
			System.arraycopy(rcv_msg, 6, DST_ADDR, 0, 6);
			UDPChatting.dstaddr = DST_ADDR;
			System.arraycopy(rcv_msg, 12, LEN_PDU, 0, 2);
			
			DSAP = rcv_msg[14];
			SSAP = rcv_msg[15];
			CONTROL[0] = rcv_msg[16];
			

			if ((byte) (CONTROL[0] & 0x80) == (byte) 0x00) {
				FRAME_TYPE_FLAG = 0;
				CONTROL[1] = rcv_msg[17];
				totallength = LEN_PDU[0] * 256 + LEN_PDU[1] * 1;
				
				seq = (byte) (CONTROL[0] & 0x7F);
				ack = (byte) (CONTROL[1] & 0x7F);
				datalength = totallength - 22;
				System.arraycopy(rcv_msg, 18, DATA, 0, datalength);
				System.arraycopy(rcv_msg, totallength - 4, CRC, 0, 4);				
				
				byte[] cache1 = new byte[totallength - 4];
				System.arraycopy(rcv_msg, 0, cache1, 0, totallength - 4);
				
				if (corrupted(CRC, cache1)) {
					System.out.println();
					System.out.println("\nSTATE : Data is comming...");
					System.out.println("Correct CRC");
					printCRC(CRC);
					// If sequence is not correct
					if (NR != seq) {
						System.out.println("\nACK is not equal!!!");
						sendACK(); // send ACK
					}
					// sequence is correct
					else {
						String result = new String(DATA).substring(0, DATA.length);
						System.out.println("\nReceived Data : " + result);
						NR = (byte) (seq+1);
						sendACK(); // send ACK
					}
				}
				// If error detected
				else {
					System.out.println("Error Detected!!!");
					sendNAK();
					printField(rcv_msg);
				}

			} else if ((byte) (CONTROL[0] & 0xC0) == (byte) 0x80) {
				FRAME_TYPE_FLAG = 10;
				
				CONTROL[1] = rcv_msg[17];
				totallength = LEN_PDU[0] * 256 + LEN_PDU[1] * 1;
				
				ack = (byte) (CONTROL[1] & 0x7F);

				byte[] cache2 = new byte[18];
				System.arraycopy(rcv_msg, 0, cache2, 0, 18);
				System.arraycopy(rcv_msg, 18, CRC, 0, 4);

				switch ((byte) (CONTROL[0] & 0x30)) {
				case ((byte) 0x00):
					CONTROL_TYPE_FLAG = 11;
					break;
				case ((byte) 0x20):
					CONTROL_TYPE_FLAG = 12;
					break;
				case ((byte) 0x10):
					CONTROL_TYPE_FLAG = 13;
					break;
				default:
					break;
				}

				// ACK
				if (CONTROL_TYPE_FLAG == 11) {
					if (corrupted(CRC, cache2)) {
						System.out.println();
						System.out.println("\nSTATE : ACK is comming...");
						System.out.println("Correct CRC");
						printCRC(CRC);
						// ACK sequence is correct
						if (ack == UDPChatting.NS) {
							System.out.println("\nACK(no." + ack + ") Received!!!");
							seq = UDPChatting.NS;
							signal.ACKnotifying();
							printField(rcv_msg);
						} // ACK sequence is not correct
						else {
							System.out.println("\nACK and seq are not equal!!!");
							printField(rcv_msg);
						}
					} else {
						System.out.println("CRC is not correct!!!");
						sendNAK();
						printField(rcv_msg);
					}
				}
				// NAK
				else if (CONTROL_TYPE_FLAG == 13) {
					if (corrupted(CRC, cache2)) {
						System.out.println("Data corrupted!");
						printField(rcv_msg);
					} else {
						System.out.println("Error Detected!!!");
						sendNAK();
						printField(rcv_msg);
					}
				}
				// RNR
				else if (CONTROL_TYPE_FLAG == 12) {

				}

			} else {
				FRAME_TYPE_FLAG = 20;
				totallength = LEN_PDU[0] * 256 + LEN_PDU[1] * 1;
				
				byte[] cache3 = new byte[17];
				System.arraycopy(rcv_msg, 0, cache3, 0, 17);
				System.arraycopy(rcv_msg, 17, CRC, 0, 4);
				
				switch ((byte) (CONTROL[0] & 0x37)) {
				
				case ((byte) 0x30):
					System.out.println("CONTROL_TYPE: UI");
					CONTROL_TYPE_FLAG = 21;
					break;
				case ((byte) 0x36):
					CONTROL_TYPE_FLAG = 22;
					break;
				case ((byte) 0x02):
					CONTROL_TYPE_FLAG = 23;
					break;
				case ((byte) 0x21):
					System.out.println("CONTROL_TYPE: FRMR");
					CONTROL_TYPE_FLAG = 24;
					break;
				case ((byte) 0x06):
					CONTROL_TYPE_FLAG = 25;
					break;
				case ((byte) 0x35):
					System.out.println("CONTROL_TYPE: XID");
					CONTROL_TYPE_FLAG = 26;
					break;
				default:
					System.out.println("???");
					break;
				}

				if (CONTROL_TYPE_FLAG == 21) {

				} else if (CONTROL_TYPE_FLAG == 22) {
					/*
					//check for received SABME
					for(int i =0 ; i < 21 ; i++){
					System.out.printf("[%02X]" , rcv_msg[i]);
					}
					System.out.println();
					*/
					if(corrupted(CRC,cache3)) {
						System.out.println("Received SABME");
						signal.SABMEnotifying();
						sendUA();
					}else {
						System.out.println("Received SABME. BUT ERROR DETECTED!!!");
					}
				} else if (CONTROL_TYPE_FLAG == 23) {
					if(corrupted(CRC,cache3)) {
						System.out.println("\nReceived DISC");
						signal.DISCnotifying();
						sendUA();
						System.out.println("Port Closed!! Communication End!!!");
						graceout();
					}else {
						System.out.println("Received SABME. BUT ERROR DETECTED!!!");
					}

				} else if (CONTROL_TYPE_FLAG == 24) {

				} else if (CONTROL_TYPE_FLAG == 25) {
					if(cnt == 0) {
						System.out.println("Received UA(From SABME)!!!");
						signal.SABMEUAnotifying();
						cnt++;
					}else {
						System.out.println("Received UA(From DISC)!!!");
						signal.DISCUAnotifying();
						System.out.println("Port Closed!! Communication End!!!");
						UDPChatting.graceout();
					}
				} else if (CONTROL_TYPE_FLAG == 26) {

				}

			}
		}
	}

	// Error Detection
	public boolean corrupted(byte[] crc, byte[] msg) {

		CRC c = new CRC();
		byte[] sibal = new byte[msg.length];
		System.arraycopy(msg, 0, sibal, 0, msg.length);

		byte[] check2 = new byte[4];
		check2 = c.CRCmake(sibal);

		return (new CRC().isequalCRC(crc, check2));
	}

	// monitoring
	public void printField(byte[] frame) {
		switch (FRAME_TYPE_FLAG) {
		case 0:
			System.out.println("FrameType(I). N(S):" + NS + " N(R):" + NR);
			break;
		case 10:
			if (CONTROL_TYPE_FLAG == 11) {
				System.out.println("FrameType(S)(ACK). N(S):" + NS + " N(R):" + ack);
				break;
			} else if (CONTROL_TYPE_FLAG == 12) {
				System.out.println("FrameType(S)(NAK) , N(S): " + NS + " N(R): " + NR);
				break;
			} else if (CONTROL_TYPE_FLAG == 13) {
				System.out.println("FrameType(S)(RNR) , N(S): " + NS + " N(R): " + NR);
				break;
			}
		case 20:
			if (CONTROL_TYPE_FLAG == 21) {
				System.out.println("FrameType(U)(UI)");
				break;
			} else if (CONTROL_TYPE_FLAG == 22) {
				System.out.println("FrameType(U)(SABME)");
				break;
			} else if (CONTROL_TYPE_FLAG == 23) {
				System.out.println("FrameType(U)(DISC)");
				break;
			} else if (CONTROL_TYPE_FLAG == 24) {
				System.out.println("FrameType(U)(FRMR)");
				break;
			} else if (CONTROL_TYPE_FLAG == 25) {
				break;
			} else if (CONTROL_TYPE_FLAG == 26) {
				System.out.println("FrameType(U)(XID)");
				break;
			}
		}
	}

	public void printCRC(byte[] CRC) {
		System.out.print("CRC:");
		System.out.printf("[%02X]", CRC[0]);
		System.out.printf("[%02X]", CRC[1]);
		System.out.printf("[%02X]", CRC[2]);
		System.out.printf("[%02X]", CRC[3]);
	}

	// send ACK 
	public void sendACK() {
		System.out.println("STATE : Sending ACK");
		CRC d = new CRC();
		byte[] lenpdu = new byte[2];
		lenpdu[0] = (byte) 0x00;
		lenpdu[1] = (byte) 0x16;
		byte[] ACK = new byte[22];
		byte[] ack = new byte[2];
		ack[0] = (byte) 0x80;
		ack[1] = (byte) ((byte) 0x7F & NR);

		ACK = new Framing(DST_ADDR, SRC_ADDR, lenpdu, dsap, ssap, ack).framing();

		
		byte[] crctest2 = new byte[4];
		byte[] temp = new byte[18];
		System.arraycopy(ACK, 0, temp, 0, 18);
		crctest2 = d.CRCmake(temp);

		DatagramPacket ack_packet = new DatagramPacket(ACK, ACK.length, UDPChatting.remoteaddr, UDPChatting.remoteport);
		try {
			socket.send(ack_packet);
		} catch (IOException e) {
			e.printStackTrace();
		}
		System.out.println("FrameType(S)(ACK). N(S):" + NS + " N(R):" + ack[1]);
		printCRC(crctest2);
		System.out.println();
	}

	// send NAK
	public void sendNAK() {
		System.out.println("STATE : Sending NAK");
		byte[] lenpdu = new byte[2];
		lenpdu[0] = (byte) 0x00;
		lenpdu[1] = (byte) 0x16;
		byte[] NAK = new byte[22];
		byte[] nak = new byte[2];
		nak[0] = (byte) 0x90;
		nak[1] = (byte) ((byte) 0x7F & NR);

		NAK = new Framing(DST_ADDR, SRC_ADDR, lenpdu, dsap, ssap, nak).framing();
		DatagramPacket nak_packet = new DatagramPacket(NAK, NAK.length, UDPChatting.remoteaddr, UDPChatting.remoteport);
		try {
			socket.send(nak_packet);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	// send UA 
	public void sendUA() {
		System.out.println("STATE : Sending UA");
		CRC d = new CRC();
		byte[] lenpdu = new byte[2];
		lenpdu[0] = (byte) 0x00;
		lenpdu[1] = (byte) 0x15;
		byte[] UA = new byte[21];
		byte ua = (byte) 0xC6;



		UA = new Framing(DST_ADDR, SRC_ADDR, lenpdu, dsap, ssap, ua).framing();
		// check for received msg
		System.out.println("UA");
	    for(int i =0; i<518; i++){
	    	System.out.printf("[%02X]",UA[i]);
	    }
		System.out.println();
		
		
		byte[] crctest2 = new byte[4];
		byte[] temp = new byte[17];
		System.arraycopy(UA, 0, temp, 0, 17);
		crctest2 = d.CRCmake(temp);

		DatagramPacket ua_packet = new DatagramPacket(UA, UA.length, UDPChatting.remoteaddr, UDPChatting.remoteport);
		try {
			socket.send(ua_packet);
		} catch (IOException e) {
			System.out.println();
			e.printStackTrace();
		}
	}

	public void graceout() {
		temp = false;
	}

}
