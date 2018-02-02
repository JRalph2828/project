package UDP_Chatting;

import java.io.*;
import java.net.*;

public class UDP_Chatting {

	final static int MAXBUFFER = 512; // MAXINUM OF BUFFER
	static RcvThread rcvThread; // RcvThread
	public static DatagramSocket socket; // socket
	public static DatagramPacket packet; // packet
	public static DatagramPacket ackpacket; // ACKpacket
	public static InetAddress remoteaddr; // remote IP address
	public static int remoteport = 0; // remote PORTnumber
	public InetAddress myinetaddr; // my IPaddress
	public static int myport = 0; // my PORTnumber
	static Signaling signal = new Signaling(); // signal
	static Timeout ticks; // Timeout variable
	public static boolean check;	// boolean variable for retransmission
	public static String sendMessage;		// send message
	public static String ackMessage = "ACK";		// ACK
	public static byte buffer[] = new byte[512];

	// main thread is send thread
	public static void main(String[] args) {
		// if argc == 2 , remote IP , PORT
		if (args.length == 2) {
			remoteport = Integer.parseInt(args[1]);
			try {
				remoteaddr = InetAddress.getByName(args[0]);
			} catch (UnknownHostException e) {
				System.out.println("Error on port" + remoteport);
				e.printStackTrace();
			}
			// if argc == 1 ,my PORT
		} else if (args.length == 1) {
			myport = Integer.parseInt(args[0]);
		} else {
			System.out.println("Usage: args must be localhost port or port");
			System.exit(0);
		}
		
		try {
			// create new socket
			if (myport == 0) {
				socket = new DatagramSocket();
			} else {
				socket = new DatagramSocket(myport);
			}
			System.out.println("Datageam socket is created");

			ticks = new Timeout(); // create Timeout instance 
			rcvThread = new RcvThread(socket, signal); // create RcvThread instance
			rcvThread.start(); // run RcvThread 

			//DatagramPacket packet;
			BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
			
			
			while (true) {
				if(check){
					// if RcvThread received ACK then reset Timeout
					if (Signaling.ACKNOTIFY) {
						ticks.TimeoutReset(0);
					}
					else
					{
						// if RcvThread didn't receive ACK then retransmit sendMessage and set Timeout
						System.out.println("Retransmit!!(Message : " + sendMessage + " )");
						socket.send(packet);
						ticks.TimeoutSet(0, 3000, signal);
					}
				}
				check = true;	// set check value true for check if retransmit or not 
				// input data by keyboard
				System.out.print("Input Data : ");
				
				sendMessage = br.readLine();
				
				// save message in buffer				
				buffer = sendMessage.getBytes();

				// if correct IP address
				if ((remoteaddr != null)) {
					// create packet
					packet = new DatagramPacket(buffer, buffer.length, remoteaddr, remoteport);
					// transmit packet
					socket.send(packet);
					// set Timeout 3000ms
					ticks.TimeoutSet(0, 3000, signal);
					// RcvThread is waiting for ACK
					signal.waitingACK();
					
				} else {
					// incorrect IP address go here 
					System.out.println("remote IP address is not appropriate");
				}
			}
		}catch(IOException e) {
			System.out.println(e);
		}
		rcvThread.graceout();
		System.out.println("grace out called");
		socket.close();
	}
}
