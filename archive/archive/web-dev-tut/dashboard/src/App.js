import './App.css'
import Temp from './temperature/Temp';
import Dist from './distance/Dist';
import Humid from './humidity/Humid';

function App() {
  return (
    <div className="App">
      <Temp/>
      <Dist/>
      <Humid/>
    </div>
  )
}

export default App;
