package main

import (
	"golang.org/x/net/websocket"
	"flag"
	"fmt"
	"net/http"
)

func main()  {
	var frontend,backend int
	flag.IntVar(&frontend, "l", 8088, "frontend, serve websocket")
	flag.IntVar(&backend, "b", 8080, "backend, which fetch flv stream from")
	flag.Parse()

	fmt.Println(fmt.Sprintf("Transmux http://:%v/* to ws://:%v/*", backend, frontend))
	http.Handle("/", websocket.Handler(func(c *websocket.Conn){
		r := c.Request()
		url := fmt.Sprintf("http://127.0.0.1:%v%v", backend, r.RequestURI)
		fmt.Println("Proxy client", r.RemoteAddr, "to", url)
		defer c.Close()

		flv,err := http.Get(url)
		if err != nil {
			fmt.Println("Connect backend", url, "failed, err is", err)
			return
		}
		defer flv.Body.Close()

		b := make([]byte, 1024)
		for {
			var n int
			if n,err = flv.Body.Read(b); err != nil {
				fmt.Println("Recv from backend failed, err is", err)
				return
			}

			if err = websocket.Message.Send(c, b[0:n]); err != nil {
				fmt.Println("Send to ws failed, err is", err)
				return
			}

			//fmt.Println("Transmux", n, "bytes")
		}
	}))
	if err := http.ListenAndServe(fmt.Sprintf(":%v", frontend), nil); err != nil {
		fmt.Println("Serve failed, err is", err)
		return
	}
}