const express = require('express') 
const webpush = require('web-push')
const cors = require('cors')
const bodyParser = require('body-parser')

const SERVER_URL = 'http://127.0.0.1:4000/save-subscription';
const publicKey = 'BD6tU_zgNCEYlu9cIb0wc1UpbU4XOZU3XjGBYy5UlMYHCX-ttCWNUhwi8WnU-yA3QaEiVJ4hTS3lx7Q7lqm6o2Q';
const app = express() 

const vapidKeys = {
  publicKey: 'BD6tU_zgNCEYlu9cIb0wc1UpbU4XOZU3XjGBYy5UlMYHCX-ttCWNUhwi8WnU-yA3QaEiVJ4hTS3lx7Q7lqm6o2Q',
  privateKey: '0RPxEBKnOaF03lIBhHfVkGMjafWEF5FUcmVGU1L72KY',
}

//setting our previously generated VAPID keys
webpush.setVapidDetails(
  'mailto:myuserid@email.com',
  vapidKeys.publicKey,
  vapidKeys.privateKey
)

app.use(cors()) 
app.use(bodyParser.json()) 
const port = 4000 
app.get('/', (req, res) => res.send('Hello World!')) 

const dummyDb = { subscription: null } //dummy in memory store

const saveToDatabase = async subscription => {
  // Since this is a demo app, I am going to save this in a dummy in memory store. Do not do this in your apps.
  // Here you should be writing your db logic to save it.
  dummyDb.subscription = subscription
}// The new /save-subscription endpoint
app.post('/save-subscription', async (req, res) => {
  const subscription = req.body
  await saveToDatabase(subscription) //Method to save the subscription to Database
  res.json({ message: 'success2' })
})


//function to send the notification to the subscribed device
const sendNotification = (subscription, dataToSend) => {
  webpush.sendNotification(subscription, dataToSend)
}

//route to test send notification
app.get('/send-notification', (req, res) => {
  const subscription = dummyDb.subscription //get subscription from your databse here.
  const message = 'Hello World'
  sendNotification(subscription, message)
  res.json({ message: 'message sent' })
})



app.listen(port, () => console.log(`Example app listening on port ${port}!`))
