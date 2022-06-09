import React,{useState, useEffect} from 'react'
import axios from 'axios'
import './dist.css'

const Dist = () => {
    const [info, setInfo] = useState([])
    useEffect(() => {
        const fetchData = async () => {
            try {
                const res = await axios.get('http://localhost:5000/data')
                setInfo(res.data)
                console.log(res.data)
            } catch (error) {
                console.log(error)
            }
        }
        fetchData()
    }, [info])
    
    return (
        <div className='dist'><h3>Distance</h3>{info[0]?.DISTANCE}</div>
    )
}
export default Dist