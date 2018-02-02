package UDP_Chatting;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

class RcvThread extends Thread {

	DatagramSocket socket;
	DatagramPacket packet;
	public static DatagramPacket ackpacket; // ACK packet
	public static byte buffer[] = new byte[512];
	public static String ackMessage = "ACK"; // ACK
	Signaling signal;

	// bollean variable for control RcvThread
	boolean tmp = true;

	// RcvThread create
	RcvThread(DatagramSocket socket, Signaling signal) {
		this.socket = socket;
		this.signal = signal;
	}

	// RcvThread run
	public void run() {
		// create packet
		byte buff[] = new byte[100];
		packet = new DatagramPacket(buff, buff.length);

		// if tmp is true  then waiting for packet
		while (tmp) {
			try {
				// create socket by received packet
				socket.receive(packet);
				// check for IP address and port number
				UDP_Chatting.remoteport = packet.getPort();
				UDP_Chatting.remoteaddr = packet.getAddress();

			} catch (IOException e) {
				System.out.println("IOException : " + e);
			}
			// 
			String ReceivedMessage = new String(packet.getData()).substring(0, packet.getLength());
			// if received data is ACK then call ACKnotify()
			if (ReceivedMessage.equalsIgnoreCase("ACK")) { 
				System.out.println("ACK recieved!!");
				signal.ACKnotify();
			}
			// if received data is message
			else {
				// print out received data
				System.out.println("\n Receive Message : " + ReceivedMessage);
				buffer = ackMessage.getBytes();
				ackpacket = new DatagramPacket(buffer, buffer.length, UDP_Chatting.remoteaddr, UDP_Chatting.remoteport);
				/*
				// retransmission code - set delay lager than timeout
				try {
					Thread.sleep(5000);
				} catch (InterruptedException ex) {
					Thread.currentThread().interrupt();
				}
				*/
				// when message received then transmit ACK
				try {
					System.out.println("ACK transmt!!");
					socket.send(ackpacket);
					signal.ACKnotify();
				} catch (IOException e) {
					System.out.println(e);
				}
			}
			



			// signal.ACKnotify();
		}

		// RcvThread off
		System.out.println("RcvThread off");
	}

	public void graceout() {
		tmp = false;
	}

}
