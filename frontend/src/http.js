import axios from 'axios'

const host = process.env.NODE_ENV === 'production' ? `http://${document.location.host}/` : 'http://192.168.2.38/'

export const http = axios.create({
  baseURL: host,
  headers: {
    Authorization: 'Bearer {token}'
  }
})
