package UDP_Chatting_LLC;

import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;

public class UDPChatting {

	static RcvThread rcvThread;
	public static DatagramSocket socket;
	public static DatagramPacket packet;
	public static DatagramPacket upacket;
	public static DatagramPacket ACKpacket;

	/* IP , PORT */
	public static InetAddress remoteaddr;
	public static int remoteport = 0;
	public InetAddress myinetaddr;
	public static int myport = 0;

	static Signaling signal = new Signaling(); // Signaling Instance
	static Timeout ticks;
	public static boolean check = false; // for control receive thread
	public static boolean sRun = false; // for control send thread
	public static String sendMessage; // chatting message

	/* buffer */
	public static byte buffer[] = new byte[518]; // total buffer
	public static byte ubuffer[] = new byte[21]; // buffer for SABME

	/* component of Ethernet Frame */
	public static byte[] dstaddr = new byte[6]; // destination Mac address
	public static byte[] srcaddr = new byte[6]; // source Mac address
	public static byte[] lenpdu = new byte[2]; // Length PDU
	public static byte dsap = 0; // DSAP
	public static byte ssap = 0; // SSAP
	public static byte[] control = new byte[2]; // CONTROL
	public static byte NS = 0, NR = 1; // N(S) , N(R)  
	public static int totallength = 0; // length of total frame
	public static int datalength = 0; // length of chatting message

	/* Flags */
	public static final byte FRAME_TYPE_I = 0; // I-format
	public static final byte FRAME_TYPE_S = 10; // S-format
	public static final byte CONTROL_TYPE_RR = 11; // RR(ACK)
	public static final byte CONTROL_TYPE_RNR = 12; // RNR
	public static final byte CONTROL_TYPE_NAK = 13; // NAK
	public static final byte FRAME_TYPE_U = 20; // U-format
	public static final byte CONTROL_TYPE_UI = 21; // UI
	public static final byte CONTROL_TYPE_SABME = 22; // SABME
	public static final byte CONTROL_TYPE_DISC = 23; // DISC
	public static final byte CONTROL_TYPE_FRMR = 24; // FRMR
	public static final byte CONTROL_TYPE_UA = 25; // UA
	public static final byte CONTROL_TYPE_XID = 26; // XID
	public static byte FRAME_TYPE_FLAG = 98;
	public static byte CONTROL_TYPE_FLAG = 99;

	// return local mac address
	public static byte[] getLocalMacAddr() {
		// get local ip address
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
			e.printStackTrace();
		}

		// if network interface is not null
		if (netif != null) {
			// get mac address
			try {
				mac = netif.getHardwareAddress();
			} catch (SocketException e) {
				e.printStackTrace();
			}
		}
		return mac;
	}

	public static String byteArrayToHex(byte[] a) {
		StringBuilder sb = new StringBuilder();
		for (final byte b : a)
			sb.append(String.format("%02x", b & 0xff));
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

	// print CRC
	public static void printCRC(byte[] cache) {
		CRC c = new CRC();
		byte[] crctest = new byte[4];
		crctest = c.CRCmake(cache);

		System.out.print("CRC:");
		System.out.printf("[%02X]", crctest[0]);
		System.out.printf("[%02X]", crctest[1]);
		System.out.printf("[%02X]", crctest[2]);
		System.out.printf("[%02X]", crctest[3]);

	}

	// monitoring
	public static void printField(byte[] frame) {

		byte[] cache = new byte[totallength - 4];
		System.arraycopy(frame, 0, cache, 0, totallength - 4);

		if (FRAME_TYPE_FLAG == 0) {
			System.out.println("\nFrameType(I). N(S):" + NS + " N(R):" + NR);
			printCRC(cache);
		} else if (FRAME_TYPE_FLAG == 20) {
			switch (CONTROL_TYPE_FLAG) {
			case 21:
				System.out.println("\nFrameType(U)(UI)");
				break;
			case 22:
				System.out.println("\nFrameType(U)(SABME)");
				break;
			case 23:
				System.out.println("\nFrameType(U)(DISC)");
				break;
			case 24:
				System.out.println("\nFrameType(U)(FRMR)");
				break;
			case 25:
				System.out.println("\nFrameType(U)(UA)");
				break;
			case 26:
				System.out.println("\nFrameType(U)(XID)");
				break;
			default:
				break;
			}
		}
	}
	
	public static void graceout() {
		sRun = false;
	}

	public static void main(String[] args) throws SocketException {
		// if argc == 2 , remote IP , PORT (Client Mode)
		if (args.length == 2) {
			remoteport = Integer.parseInt(args[1]);
			try {
				remoteaddr = InetAddress.getByName(args[0]);
			} catch (UnknownHostException e) {
				System.out.println("Error on port" + remoteport);
				e.printStackTrace();
			}

			socket = new DatagramSocket();
			ticks = new Timeout(); // make Timeout instance
			rcvThread = new RcvThread(socket, signal); // make RcvThread instance
			rcvThread.start(); // run RcvThread

			for (int i = 0; i < 6; i++) {
				dstaddr[i] = (byte) 0;
			}
			srcaddr = getLocalMacAddr();
			lenpdu[0] = 0;
			lenpdu[1] = (byte)21;
			control[0] = (byte) 0xF6;
			control[1] = 0;

			ubuffer = new Framing(dstaddr, srcaddr, lenpdu, dsap, ssap, control[0]).framing();
			upacket = new DatagramPacket(ubuffer, ubuffer.length, remoteaddr, remoteport);
			/*
			// check for send SABME
		    for(int i =0; i<518; i++){
		    	System.out.printf("[%02X]",ubuffer[i]);
		    }
			System.out.println();
			*/
			
			System.out.println("Sending SABME...");
			try {
				socket.send(upacket);	// send SABME
				signal.waitingSABMEUA();	// rcv thread is waiting for UA
			} catch (IOException e) {
				e.printStackTrace();
			}
			while (true) {
				if (Signaling.SABMEUANOTIFY) {  // when UA come then run send-thread
					sRun = true;
					break;
				}
			}

			// if argc == 1 , my PORT (Server Mode)
		} else if (args.length == 1) {
			myport = Integer.parseInt(args[0]);

			for (int i = 0; i < 6; i++) {
				dstaddr[i] = (byte) 0;
			}
			srcaddr = getLocalMacAddr();
			socket = new DatagramSocket(myport);

			ticks = new Timeout(); // make Timeout instance
			rcvThread = new RcvThread(socket, signal); // make RcvThread instance
			rcvThread.start(); // run RcvThread
			signal.waitingSABME();	// rcv thread is waiting for SABME

			while (true) {
				if (Signaling.SABMENOTIFY) { // when SABME come then run send-thread
					sRun = true;
					break;
				}
			}

		} else {
			System.out.println("Usage: args must be localhost port or port");
			System.exit(0);
		}

		try {
			BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
			System.out.print("Communication Start");
			check = false;
			while (sRun) {

				System.out.println();
				// check for ACK
				if (check) {
					if (Signaling.ACKNOTIFY) {
						// if ACK received then initialize Timeout
						ticks.TimeoutReset(0);
					} else if(!Signaling.ACKNOTIFY){
						// if ACK isn't received then re-send packet and reset Timeout
						System.out.println("\nRetransmission!!(Message : " + sendMessage + " )");
						socket.send(packet);
						ticks.TimeoutSet(0, 3000, signal);
						signal.waitingACK();
					}
				}
				check = true;

				
				System.out.print("\nInput Data : ");
				sendMessage = br.readLine();

				datalength = sendMessage.getBytes().length;
				totallength = datalength + 22;
				
				// when DISC is coming, U format - DISC
				if(datalength == 4 && sendMessage.equalsIgnoreCase("DISC")) {
					lenpdu[0] = (byte)0;
					lenpdu[1] = (byte)21;
					control[0] = (byte)0xC2;
					control[1] = (byte)0;
					buffer = new Framing(dstaddr, srcaddr, lenpdu, dsap, ssap, control[0]).framing();
					packet = new DatagramPacket(buffer, buffer.length, remoteaddr, remoteport);
					System.out.println("Sending DISC...");
					socket.send(packet);
					signal.waitingDISCUA();
				} else {
					byte[] temp = ByteBuffer.allocate(4).putInt(totallength).array();
					lenpdu[0] = temp[2];
					lenpdu[1] = temp[3];
					control[0] = NS;
					NR = (byte) (NS+1);	
					control[1] = NR;

					// Framing message
					buffer = new Framing(dstaddr, srcaddr, lenpdu, dsap, ssap, control, sendMessage.getBytes()).framing();
					//check for send msg
				    for(int i =0; i<518; i++){
				    	System.out.printf("[%02X]",buffer[i]);
				    }
					System.out.println();
					
					
					FRAME_TYPE_FLAG = 0;
					// if remote IP is not NULL 
					if ((remoteaddr != null)) {
						// make packet
						packet = new DatagramPacket(buffer, buffer.length, remoteaddr, remoteport);
					// send packet
					System.out.print("STATE : Sending Data(" + sendMessage + ")...");
					socket.send(packet);
					printField(buffer);
					NS++;
					// sending packet and set Timeout as 3000ms
					ticks.TimeoutSet(0, 3000, signal);
					// RcvThread is waiting for ACK
					signal.waitingACK();
					} else {
						// remote IP is incorrect
						System.out.println("remote IP address is not appropriate");
					}
				}
			}
		}catch(IOException e) {
			System.out.println(e);
		}
		rcvThread.graceout();
		sRun=false;
		System.out.println("grace out called");
		socket.close();
	}
}

