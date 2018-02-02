

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Iterator;
import java.util.TreeMap;

public class UdpEchoMultiThreadTimerClient {

	final static int MAXBUFFER = 512;
	final static int TIMEOUT = 1000;
	final static int NORMALSEND = 0;
	final static int REPEATSEND = 1;
	static DatagramSocket socket;
	static int gPort;
	static String gIP;
	static TreeMap<Long, String> map = new TreeMap<Long, String>();

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		if (args.length != 2) {
			System.out.println("ªÁøÎπ˝ : java -jar UdpEchoMultiThreadTimerClient.jar [localhost] [port]");
			System.exit(0);
		}
		gPort = Integer.parseInt(args[1]);
		gIP = args[0];
		try {
			socket = new DatagramSocket();
			
		} catch (IOException e) {
			System.out.println(e);
		}
		receiverThread receiverThread = new receiverThread();
		receiverThread.start();
		// receiverthread

		senderThread senderThread = new senderThread(NORMALSEND, null);
		senderThread.start();
		// send thread

	}

	static class receiverThread extends Thread {

		public receiverThread() {
			// TODO Auto-generated constructor stub
		}

		public void run() {
			// TODO Auto-generated method stub
			while (true) {
				try {
					byte buffer[] = new byte[MAXBUFFER];
					DatagramPacket recv_Packet = new DatagramPacket(buffer,
							buffer.length);
					socket.receive(recv_Packet);
					

					String result = new String(recv_Packet.getData())
							.substring(0, recv_Packet.getLength());
					Iterator<Long> iter = map.keySet().iterator();
					long key = 0;
					while (iter.hasNext()) {
						key = iter.next();
						if (map.get(key).equals(result)) {
							break;
						}
					}
					if (key != 0){
						System.out.println(recv_Packet.getAddress() + " "
								+ recv_Packet.getPort());
						System.out.println("Echo data:" + result);
						map.remove(key);
						
					}

				} catch (Exception e) {
					// TODO: handle exception
					System.out.println(e);
				}
			}

		}

	}

	static public class timerThread extends Thread {
		String object;
		Long time;
		Long afterTime = null;

		public timerThread(String object, Long time) {
			// TODO Auto-generated constructor stub
			this.object = object;
			this.time = time;
		}

		@Override
		public void run() {
			// TODO Auto-generated method stub
			while (true) {
				if (map.containsKey(time)) {
					long delay = System.currentTimeMillis();
					if (afterTime != null)
						delay -= afterTime;
					else
						delay -= time;
					
					if (delay > TIMEOUT) {
						senderThread resend = new senderThread(REPEATSEND, object);
						resend.start();
						System.out.println(object + " is resent during "
								+ delay + "ms");
						afterTime = System.currentTimeMillis();
					}
					try {
						Thread.sleep(50);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				} else {
					break;
				}
			}
		}
	}

	static public class senderThread extends Thread {
		int option;
		String object;

		public senderThread(int option, String object) {
			// TODO Auto-generated constructor stub
			this.option = option;
			this.object = object;
		}

		public void run() {
			// TODO Auto-generated method stub
			InetAddress inetAddress = null;
			DatagramPacket send_Packet;
			try {
				inetAddress = InetAddress.getByName(gIP);

				if (option == NORMALSEND) {
					while (true) {
						byte buffer[] = new byte[MAXBUFFER];

						BufferedReader br = new BufferedReader(
								new InputStreamReader(System.in));
						String data = null;
						Thread.sleep(20);

						System.out.print("Input data:");
						data = br.readLine();
						if (data.length() != 0) {
							buffer = data.getBytes();
							
							send_Packet = new DatagramPacket(buffer,
									buffer.length, inetAddress, gPort);
							
							long time = System.currentTimeMillis();
							

							map.put(time, data);
							timerThread timer = new timerThread(data, time);
							timer.start();

							socket.send(send_Packet);

						}
					}
				} else {
					//send thread
					send_Packet = new DatagramPacket(object.getBytes(),
							object.getBytes().length, inetAddress, gPort);
					socket.send(send_Packet);
				}
			} catch (UnknownHostException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
}
