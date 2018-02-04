import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.stream.*;
class hashes {
    public static void main(String a[]) throws IOException
    {
        HashMap<String,Object> hs = new HashMap<>();
        long startTime = System.currentTimeMillis();
        List<String> lines = Files.readAllLines(Paths.get("book.txt"), java.nio.charset.StandardCharsets.ISO_8859_1);
        List<String> words = lines.stream()
            .flatMap(l -> Arrays.stream(l.split("\\s")))
            .filter(s -> !s.equals(""))
            // .peek(e -> System.out.println("'"+e+"'"))
            // .limit(100)
            .collect(Collectors.toList());
        long endTime = System.currentTimeMillis();
        System.out.println("Read " + words.size() + " words in "+ (endTime-startTime) + "ms");

        startTime = System.currentTimeMillis();
        for (String s: words) {
                hs.put(s, null);
        }
        endTime = System.currentTimeMillis();

        System.out.println("Hashed " + hs.size() + " words in " + (endTime-startTime) + "ms");


    }
}
