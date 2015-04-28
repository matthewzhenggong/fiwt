function checkTime( data )
T=data(:,1);
deltaT = T(1:end-1,1)-T(2:end,1);
figure;
plot(T(2:end,1),deltaT);
xlabel('T/s');
ylabel('\delta T/s');
end

