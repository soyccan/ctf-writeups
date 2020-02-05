FROM openjdk:8-jdk-alpine
COPY deps/ /deps
COPY static/ /static
COPY index.html /index.html
COPY run.sh /run.sh
COPY *.java /
COPY flag.txt /

RUN javac -cp ".:deps/*" /Frontend.java
RUN javac -cp ".:deps/*" /Backend.java
RUN javac -cp ".:deps/*" /Backend2.java

CMD ["./run.sh"]
