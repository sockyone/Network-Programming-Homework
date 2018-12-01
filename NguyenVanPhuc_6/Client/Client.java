import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.util.Scanner;

/**
 *
 * @author phuc
 */
public class Client {

    public static Scanner sc = new Scanner(System.in);
    private static Socket socket;
    public final static int BUFFER_SIZE = 16 * 1024;
    public final static String QUIT_KEY = "QUIT";

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        String host;
        int port;
        try {
            System.out.print("Address: ");
            host = sc.nextLine();
            System.out.print("Port: ");
            port = sc.nextInt();
            sc.nextLine();
            InetAddress address = InetAddress.getByName(host);
            socket = new Socket(address, port);
//            socket = new Socket("localhost", 3001);

            OutputStream os = socket.getOutputStream();
            InputStream is = socket.getInputStream();
            OutputStreamWriter osw = new OutputStreamWriter(os);
            BufferedWriter bw = new BufferedWriter(osw);
            while (true) {
                // Enter file name
                System.out.print(">>> ");
                String fileName = sc.nextLine();
                if (QUIT_KEY.equals(fileName)) {
                    socket.close();
                    break;
                }
                String sendMessage = fileName + "\n";
                bw.write(sendMessage);
                bw.flush();

                // Read file size
                DataInputStream dis = new DataInputStream(is);
                long fileSize = dis.readLong();
                // System.out.println(fileSize);
                if (fileSize == 0) {
                    System.out.println("No file on Server");
                    continue;
                }

                // Receive file from Server
                String workDir = System.getProperty("user.dir");
                OutputStream out = new FileOutputStream(workDir + "/" + fileName);
                byte[] bytes = new byte[BUFFER_SIZE];
                long count;
                long total = 0;
                while ((count = is.read(bytes)) > 0) {
                    out.write(bytes, 0, (int) count);
                    total += count;
                    if (fileSize == total) {
                        break;
                    }
                }
                out.close();
                System.out.println("Done");
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                socket.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

}
