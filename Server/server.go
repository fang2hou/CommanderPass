package main

import (
	"fmt"
	"math/rand"
	"net/http"
	"strconv"
)

// aelApi
type aelAPI struct {
	sourceLink  string
	uuidMap     map[string]string
	usernameMap map[string]string
	status      map[string]bool
	secretKey   map[string]string
}

func (a *aelAPI) getUUID(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Access-Control-Allow-Origin", r.Header.Get("Origin"))
	r.ParseForm()

	if r.Method != "GET" {
		return
	}

	userName := r.Form["username"][0]
	if a.uuidMap[userName] == "" {
		uuid := strconv.FormatInt(rand.Int63(), 16)
		a.uuidMap[userName] = uuid
		a.status[uuid] = false
		a.usernameMap[uuid] = userName
	}

	fmt.Fprint(w, a.uuidMap[userName])
}

func (a *aelAPI) getStatus(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Access-Control-Allow-Origin", r.Header.Get("Origin"))
	r.ParseForm()

	if r.Method != "GET" {
		return
	}

	uuid := r.Form["uuid"][0]
	username := a.usernameMap[uuid]

	if a.status[uuid] == true {
		fmt.Fprint(w, a.sourceLink)

		delete(a.uuidMap, username)
		delete(a.status, uuid)
		delete(a.usernameMap, uuid)

	} else {
		fmt.Fprint(w, "waiting")
	}
}

func (a *aelAPI) auth(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Access-Control-Allow-Origin", r.Header.Get("Origin"))
	r.ParseForm()

	if r.Method != "POST" {
		return
	}

	authKey := r.Form["key"][0]
	model := authKey[0:7]
	sessionUUID := authKey[7:23]
	enteredPass := authKey[23:27]

	username := a.usernameMap[sessionUUID]

	if model == "AEL0106" {
		if enteredPass == a.secretKey[username] {
			fmt.Fprint(w, "success")
			a.status[sessionUUID] = true
			return
		}
	}

	fmt.Fprint(w, "failed")
}

func (a *aelAPI) reg(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Access-Control-Allow-Origin", r.Header.Get("Origin"))
	r.ParseForm()

	if r.Method != "POST" {
		return
	}

	pass := r.Form["pass"][0]
	username := r.Form["user"][0]

	a.secretKey[username] = pass

	fmt.Fprint(w, "register - ")
	fmt.Fprint(w, username)
}

func main() {
	exAPI := aelAPI{
		sourceLink:  "https://github.com/fang2hou/EasyGA/archive/v0.4.0.zip",
		uuidMap:     make(map[string]string),
		usernameMap: make(map[string]string),
		status:      make(map[string]bool),
		secretKey:   make(map[string]string),
	}

	exAPI.secretKey["fangzhou"] = "1234"

	http.HandleFunc("/get/status", exAPI.getStatus)
	http.HandleFunc("/get/uuid", exAPI.getUUID)
	http.HandleFunc("/post/auth", exAPI.auth)
	http.HandleFunc("/post/reg", exAPI.reg)

	if err := http.ListenAndServe(":8080", nil); err != nil {
		fmt.Println("ListenAndServe err", err)
	}
}
