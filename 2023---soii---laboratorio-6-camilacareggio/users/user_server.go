package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/golang-jwt/jwt/v4"
)

// Create the JWT key used to create the signature
var jwtKey = []byte("my_secret_key")

// Create a struct to read the username and password from the request body
type Credentials struct {
	ID        int    `json:"id"`
	Username  string `json:"username"`
	Password  string `json:"password"`
	CreatedAt string `json:"created_at"`
}

// Create a struct to represent the user response
type UserResponse struct {
	UserID   string `json:"user_id"`
	Username string `json:"username"`
}

// Create a struct that will be encoded to a JWT.
type Claims struct {
	Username string `json:"username"`
	jwt.RegisteredClaims
}

func Login(w http.ResponseWriter, r *http.Request) {
	var creds Credentials
	// Get the JSON body and decode into credentials
	err := json.NewDecoder(r.Body).Decode(&creds)
	if err != nil {
		// If the structure of the body is wrong, return an HTTP error
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	// Check if the user exists in the OS
	cmd := exec.Command("id", creds.Username)
	err = cmd.Run()
	if err != nil {
		// If the command returns an error, the user does not exist
		w.WriteHeader(http.StatusUnauthorized)
		w.Write([]byte("Unauthorized!"))
		return
	}

	// Declare the expiration time of the token: 5 minutes
	expirationTime := time.Now().Add(5 * time.Minute)
	claims := &Claims{
		Username: creds.Username,
		RegisteredClaims: jwt.RegisteredClaims{
			ExpiresAt: jwt.NewNumericDate(expirationTime),
		},
	}

	// Declare the token with the algorithm used for login, and the claims
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	// Create the JWT string
	tokenString, err := token.SignedString(jwtKey)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// Sign-in successful message
	response := struct {
		Message string `json:"message"`
		Token   string `json:"token"`
	}{
		Message: "Login successful!",
		Token:   tokenString,
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	err = json.NewEncoder(w).Encode(response)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// Create an SSH session
	cmd = exec.Command("gnome-terminal", "--", "ssh", creds.Username+"@dashboard.com")
	err = cmd.Start()
	if err != nil {
		log.Println("Error starting SSH session:", err)
	}
}

func CreateUser(w http.ResponseWriter, r *http.Request) {
	var newUser Credentials
	// Get the JSON body and decode into a new user
	err := json.NewDecoder(r.Body).Decode(&newUser)
	if err != nil {
		// If the structure of the body is wrong, return an HTTP error
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	// Check if the user already exists in the OS
	cmd := exec.Command("id", "-u", newUser.Username)
	err = cmd.Run()
	if err == nil {
		w.WriteHeader(http.StatusConflict)
		w.Write([]byte("User already exists in the OS"))
		return
	}

	// Set the created_at field in the newUser struct
	createdAt := time.Now().Format("2006-01-02 15:04:05")
	newUser.CreatedAt = createdAt

	// Run the command to create the user in the operating system
	cmd = exec.Command("sudo", "useradd", newUser.Username)
	err = cmd.Run()
	if err != nil {
		log.Println("Error creating user:", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// Run the command to set the password for the user
	cmd = exec.Command("sudo", "passwd", newUser.Username)
	cmd.Stdin = strings.NewReader(fmt.Sprintf("%s\n%s\n", newUser.Password, newUser.Password))
	err = cmd.Run()
	if err != nil {
		log.Println("Error setting password:", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// Assign ID to new user
	cmd = exec.Command("id", "-u", newUser.Username)
	out, err := cmd.Output()
	if err != nil {
		log.Println("Error getting user ID:", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	newUser.ID, err = strconv.Atoi(strings.TrimSpace(string(out)))
	if err != nil {
		log.Println("Error converting user ID:", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// Create the response object
	response := struct {
		ID        int    `json:"id"`
		Username  string `json:"username"`
		CreatedAt string `json:"created_at"`
	}{
		ID:        newUser.ID,
		Username:  newUser.Username,
		CreatedAt: newUser.CreatedAt,
	}

	// Encode the response object to JSON and write it to the response
	err = json.NewEncoder(w).Encode(response)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	w.Write([]byte("User created successfully!"))
}

func ListAllUsers(w http.ResponseWriter, r *http.Request) {
	// Set the response content type to JSON
	w.Header().Set("Content-Type", "application/json")

	var cantUsuarios int
	var mensajeLog string

	// Run the command to get the count of users
	comandoCant := "cat /etc/passwd | grep '/home' | cut -d: -f1 | wc -l"
	outputCant, err := exec.Command("sh", "-c", comandoCant).Output()
	if err != nil {
		log.Println("Error running command:", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// Parse the output to get the count of users
	cantUsuarios, err = strconv.Atoi(strings.TrimSpace(string(outputCant)))
	if err != nil {
		log.Println("Error parsing user count:", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// Run the command to get the list of users
	comandoLista := "cat /etc/passwd | grep '/home' | cut -d: -f1"
	outputLista, err := exec.Command("sh", "-c", comandoLista).Output()
	if err != nil {
		log.Println("Error running command:", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// Run the command to get the list of user IDs
	comandoId := "cat /etc/passwd | grep '/home/' | cut -d: -f3"
	outputId, err := exec.Command("sh", "-c", comandoId).Output()
	if err != nil {
		log.Println("Error running command:", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// Split the command outputs into slices
	listaUsuarios := strings.Fields(string(outputLista))
	idUsuarios := strings.Fields(string(outputId))

	var arrayUsuarios []UserResponse

	// Iterate over the users and create a UserResponse for each
	for i := 0; i < cantUsuarios; i++ {
		userResponse := UserResponse{
			UserID:   idUsuarios[i],
			Username: listaUsuarios[i],
		}
		arrayUsuarios = append(arrayUsuarios, userResponse)
	}

	// Log the number of listed users
	mensajeLog = "Usuarios listados: " + strconv.Itoa(cantUsuarios)
	log.Println(mensajeLog)

	// Encode the array of users into JSON and write it to the response
	err = json.NewEncoder(w).Encode(arrayUsuarios)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		log.Println("Error encoding JSON response:", err)
		return
	}
}

func Logout(w http.ResponseWriter, r *http.Request) {
	// immediately clear the token cookie
	http.SetCookie(w, &http.Cookie{
		Name:    "token",
		Expires: time.Now(),
	})
}

func ValidateToken(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		// Extract the token from the Authorization header
		authHeader := r.Header.Get("Authorization")
		if authHeader == "" {
			http.Error(w, "Unauthorized! Missing token", http.StatusUnauthorized)
			return
		}
		tokenString := strings.TrimPrefix(authHeader, "Bearer ")

		// Parse and validate the token
		token, err := jwt.ParseWithClaims(tokenString, &Claims{}, func(token *jwt.Token) (interface{}, error) {
			return jwtKey, nil
		})

		if err != nil || !token.Valid {
			// Token is invalid
			http.Error(w, "Unauthorized! Invalid token", http.StatusUnauthorized)
			return
		}

		// Token is valid, proceed to the next handler
		next(w, r)
	}
}

func main() {
	http.HandleFunc("/api/users/login", Login)
	http.HandleFunc("/api/users/createuser", CreateUser)
	http.HandleFunc("/api/users/listall", ValidateToken(ListAllUsers))
	http.HandleFunc("/api/users/logout", Logout)

	// start the server on port 8000
	log.Println("Iniciando el servidor en http://localhost:8000")
	log.Fatal(http.ListenAndServe(":8000", nil))
}
