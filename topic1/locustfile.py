from locust import HttpUser, task

class SimpleUser(HttpUser):
    host = "http://127.0.0.1:8080"
    @task
    def index(self):
        self.client.get("/")
